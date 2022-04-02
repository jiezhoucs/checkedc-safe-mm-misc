/* fdwatch.c - fd watcher routines, either select() or poll()
**
** Copyright � 1999,2000 by Jef Poskanzer <jef@mail.acme.com>.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <syslog.h>
#include <fcntl.h>

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifdef HAVE_POLL_H
#include <poll.h>
#else /* HAVE_POLL_H */
#ifdef HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif /* HAVE_SYS_POLL_H */
#endif /* HAVE_POLL_H */

#ifdef HAVE_SYS_DEVPOLL_H
#include <sys/devpoll.h>
#ifndef HAVE_DEVPOLL
#define HAVE_DEVPOLL
#endif /* !HAVE_DEVPOLL */
#endif /* HAVE_SYS_DEVPOLL_H */

#ifdef HAVE_SYS_EVENT_H
#include <sys/event.h>
#endif /* HAVE_SYS_EVENT_H */

#include "fdwatch.h"

#ifdef HAVE_SELECT
#ifndef FD_SET
#define NFDBITS         32
#define FD_SETSIZE      32
#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)      bzero((char*)(p), sizeof(*(p)))
#endif /* !FD_SET */
#endif /* HAVE_SELECT */

static int nfiles;
static long nwatches;
static int nreturned, next_ridx;
static mm_array_ptr<int> fd_rw;
static mm_array_ptr<mm_ptr<void>> fd_data;

#ifdef HAVE_KQUEUE

#define WHICH                  "kevent"
#define INIT( nf )         kqueue_init( nf )
#define ADD_FD( fd, rw )       kqueue_add_fd( fd, rw )
#define DEL_FD( fd )           kqueue_del_fd( fd )
#define WATCH( timeout_msecs ) kqueue_watch( timeout_msecs )
#define CHECK_FD( fd )         kqueue_check_fd( fd )
#define GET_FD( ridx )         kqueue_get_fd( ridx )

static int kqueue_init( int nf );
static void kqueue_add_fd( int fd, int rw );
static void kqueue_del_fd( int fd );
static int kqueue_watch( long timeout_msecs );
static int kqueue_check_fd( int fd );
static int kqueue_get_fd( int ridx );

#else /* HAVE_KQUEUE */
# ifdef HAVE_DEVPOLL

#define WHICH                  "devpoll"
#define INIT( nf )         devpoll_init( nf )
#define ADD_FD( fd, rw )       devpoll_add_fd( fd, rw )
#define DEL_FD( fd )           devpoll_del_fd( fd )
#define WATCH( timeout_msecs ) devpoll_watch( timeout_msecs )
#define CHECK_FD( fd )         devpoll_check_fd( fd )
#define GET_FD( ridx )         devpoll_get_fd( ridx )

static int devpoll_init( int nf );
static void devpoll_add_fd( int fd, int rw );
static void devpoll_del_fd( int fd );
static int devpoll_watch( long timeout_msecs );
static int devpoll_check_fd( int fd );
static int devpoll_get_fd( int ridx );

# else /* HAVE_DEVPOLL */
#  ifdef HAVE_POLL

#define WHICH                  "poll"
#define INIT( nf )         poll_init( nf )
#define ADD_FD( fd, rw )       poll_add_fd( fd, rw )
#define DEL_FD( fd )           poll_del_fd( fd )
#define WATCH( timeout_msecs ) poll_watch( timeout_msecs )
#define CHECK_FD( fd )         poll_check_fd( fd )
#define GET_FD( ridx )         poll_get_fd( ridx )

static int poll_init( int nf );
static void poll_add_fd( int fd, int rw );
static void poll_del_fd( int fd );
static int poll_watch( long timeout_msecs );
static int poll_check_fd( int fd );
static int poll_get_fd( int ridx );

#  else /* HAVE_POLL */
#   ifdef HAVE_SELECT

#define WHICH                  "select"
#define INIT( nf )         select_init( nf )
#define ADD_FD( fd, rw )       select_add_fd( fd, rw )
#define DEL_FD( fd )           select_del_fd( fd )
#define WATCH( timeout_msecs ) select_watch( timeout_msecs )
#define CHECK_FD( fd )         select_check_fd( fd )
#define GET_FD( ridx )         select_get_fd( ridx )

static int select_init( int nf );
static void select_add_fd( int fd, int rw );
static void select_del_fd( int fd );
static int select_watch( long timeout_msecs );
static int select_check_fd( int fd );
static int select_get_fd( int ridx );

#   endif /* HAVE_SELECT */
#  endif /* HAVE_POLL */
# endif /* HAVE_DEVPOLL */
#endif /* HAVE_KQUEUE */


/* Routines. */

/* Figure out how many file descriptors the system allows, and
** initialize the fdwatch data structures.  Returns -1 on failure.
*/
int
fdwatch_get_nfiles( void )
    {
    int i;
#ifdef RLIMIT_NOFILE
    struct rlimit rl;
#endif /* RLIMIT_NOFILE */

    /* Figure out how many fd's we can have. */
    nfiles = getdtablesize();
#ifdef RLIMIT_NOFILE
    /* If we have getrlimit(), use that, and attempt to raise the limit. */
    if ( getrlimit( RLIMIT_NOFILE, &rl ) == 0 )
	{
	nfiles = rl.rlim_cur;
	if ( rl.rlim_max == RLIM_INFINITY )
	    rl.rlim_cur = 8192;         /* arbitrary */
	else if ( rl.rlim_max > rl.rlim_cur )
	    rl.rlim_cur = rl.rlim_max;
	if ( setrlimit( RLIMIT_NOFILE, &rl ) == 0 )
	    nfiles = rl.rlim_cur;
	}
#endif /* RLIMIT_NOFILE */

#if defined(HAVE_SELECT) && ! ( defined(HAVE_POLL) || defined(HAVE_DEVPOLL) || defined(HAVE_KQUEUE) )
    /* If we use select(), then we must limit ourselves to FD_SETSIZE. */
    nfiles = MIN( nfiles, FD_SETSIZE );
#endif /* HAVE_SELECT && ! ( HAVE_POLL || HAVE_DEVPOLL || HAVE_KQUEUE ) */

    /* Initialize the fdwatch data structures. */
    nwatches = 0;
    fd_rw = mm_array_alloc<int>(sizeof(int) * nfiles);
    fd_data = mm_array_alloc<mm_ptr<void>>(sizeof(mm_ptr<void>) * nfiles);
    if (fd_rw == NULL || fd_data == NULL) return -1;
    for ( i = 0; i < nfiles; ++i )
	fd_rw[i] = -1;
    if ( INIT( nfiles ) == -1 )
	return -1;

    return nfiles;
    }


/* Add a descriptor to the watch list.  rw is either FDW_READ or FDW_WRITE.  */
void fdwatch_add_fd(int fd, mm_ptr<void> client_data, int rw) {
    if ( fd < 0 || fd >= nfiles || fd_rw[fd] != -1 )
	{
	syslog( LOG_ERR, "bad fd (%d) passed to fdwatch_add_fd!", fd );
	return;
	}
    ADD_FD( fd, rw );
    fd_rw[fd] = rw;
    fd_data[fd] = client_data;
}

/* Remove a descriptor from the watch list. */
void
fdwatch_del_fd( int fd ) {
    if ( fd < 0 || fd >= nfiles || fd_rw[fd] == -1 )
	{
	syslog( LOG_ERR, "bad fd (%d) passed to fdwatch_del_fd!", fd );
	return;
	}
    DEL_FD( fd );
    fd_rw[fd] = -1;
    fd_data[fd] = NULL;
    }

/* Do the watch.  Return value is the number of descriptors that are ready,
** or 0 if the timeout expired, or -1 on errors.  A timeout of INFTIM means
** wait indefinitely.
*/
int
fdwatch( long timeout_msecs )
    {
    ++nwatches;
    nreturned = WATCH( timeout_msecs );
    next_ridx = 0;
    return nreturned;
    }


/* Check if a descriptor was ready. */
int
fdwatch_check_fd( int fd ) {
    if ( fd < 0 || fd >= nfiles || fd_rw[fd] == -1 )
	{
	syslog( LOG_ERR, "bad fd (%d) passed to fdwatch_check_fd!", fd );
	return 0;
	}
    return CHECK_FD( fd );
}


mm_ptr<void> fdwatch_get_next_client_data( void )
{
    int fd;

    if ( next_ridx >= nreturned )
        return create_invalid_mm_ptr<void>(-1);
    fd = GET_FD( next_ridx++ );
    if ( fd < 0 || fd >= nfiles )
	return NULL;
    return fd_data[fd];
    }

/* Generate debugging statistics syslog message. */
void
fdwatch_logstats( long secs )
    {
    if ( secs > 0 )
	syslog(
	    LOG_NOTICE, "  fdwatch - %ld %ss (%g/sec)",
	    nwatches, WHICH, (float) nwatches / secs );
    nwatches = 0;
    }


#ifdef HAVE_KQUEUE

static int maxkqevents;
static mm_array_ptr<struct kevent> kqevents = NULL;
static int nkqevents;
static mm_array_ptr<struct kevent> kqrevents = NULL;
static mm_array_ptr<int> kqrfdidx = NULL;
static int kq;


static int
kqueue_init( int nf )
    {
    kq = kqueue();
    if ( kq == -1 )
	return -1;
    maxkqevents = nf * 2;
    kqevents = MM_ARRAY_ALLOC(struct kevent,  maxkqevents);
    kqrevents = MM_ARRAY_ALLOC(struct kevent, nf);
    kqrfdidx = MM_ARRAY_ALLOC(int, nf);
    if ( kqevents == NULL || kqrevents == NULL || kqrfdidx == NULL )
	return -1;
    (void) mm_memset( kqevents, 0, sizeof(struct kevent) * maxkqevents );
    (void) mm_memset( kqrfdidx, 0, sizeof(int) * nf );
    return 0;
    }


static void
kqueue_add_fd( int fd, int rw )
    {
    if ( nkqevents >= maxkqevents )
	{
	syslog( LOG_ERR, "too many kqevents in kqueue_add_fd!" );
	return;
	}
    kqevents[nkqevents].ident = fd;
    kqevents[nkqevents].flags = EV_ADD;
    switch ( rw )
	{
	case FDW_READ: kqevents[nkqevents].filter = EVFILT_READ; break;
	case FDW_WRITE: kqevents[nkqevents].filter = EVFILT_WRITE; break;
	default: break;
	}
    ++nkqevents;
    }


static void
kqueue_del_fd( int fd )
    {
    if ( nkqevents >= maxkqevents )
	{
	syslog( LOG_ERR, "too many kqevents in kqueue_del_fd!" );
	return;
	}
    kqevents[nkqevents].ident = fd;
    kqevents[nkqevents].flags = EV_DELETE;
    switch (fd_rw[fd])
	{
	case FDW_READ: kqevents[nkqevents].filter = EVFILT_READ; break;
	case FDW_WRITE: kqevents[nkqevents].filter = EVFILT_WRITE; break;
	}
    ++nkqevents;
    }


static int
kqueue_watch( long timeout_msecs )
    {
    int i, r;

    if ( timeout_msecs == INFTIM )
	r = kevent(
	    kq, kqevents, nkqevents, kqrevents, nfiles, (struct timespec*) 0 );
    else
	{
	struct timespec ts;
	ts.tv_sec = timeout_msecs / 1000L;
	ts.tv_nsec = ( timeout_msecs % 1000L ) * 1000000L;
	r = kevent( kq, kqevents, nkqevents, kqrevents, nfiles, &ts );
	}
    nkqevents = 0;
    if ( r == -1 )
	return -1;

    for ( i = 0; i < r; ++i )
	kqrfdidx[kqrevents[i].ident] = i;

    return r;
    }


static int
kqueue_check_fd( int fd )
    {
    int ridx = kqrfdidx[fd];

    if ( ridx < 0 || ridx >= nfiles )
	{
	syslog( LOG_ERR, "bad ridx (%d) in kqueue_check_fd!", ridx );
	return 0;
	}
    if ( ridx >= nreturned )
	return 0;
    if ( kqrevents[ridx].ident != fd )
	return 0;
    if ( kqrevents[ridx].flags & EV_ERROR )
	return 0;
    switch ( fd_rw[fd] )
	{
	case FDW_READ: return kqrevents[ridx].filter == EVFILT_READ;
	case FDW_WRITE: return kqrevents[ridx].filter == EVFILT_WRITE;
	default: return 0;
	}
    }


static int
kqueue_get_fd( int ridx )
    {
    if ( ridx < 0 || ridx >= nfiles )
	{
	syslog( LOG_ERR, "bad ridx (%d) in kqueue_get_fd!", ridx );
	return -1;
	}
    return kqrevents[ridx].ident;
    }

#else /* HAVE_KQUEUE */


# ifdef HAVE_DEVPOLL

static int maxdpevents;
static mm_array_ptr<struct pollfd> dpevents = NULL;
static int ndpevents;
static mm_array_ptr<struct pollfd> dprevents = NULL;
static mm_array_ptr<int> dp_rfdidx = NULL;
static int dp;


static int
devpoll_init( int nf )
    {
    dp = open( "/dev/poll", O_RDWR );
    if ( dp == -1 )
	return -1;
    (void) fcntl( dp, F_SETFD, 1 );
    maxdpevents = nf * 2;
    dpevents = MM_ARRAY_ALLOC( struct pollfd, maxdpevents );
    dprevents = MM_ARRAY_ALLOC(struct pollfd,  nf );
    dp_rfdidx = MM_ARRAY_ALLOC( int, nf );
    if ( dpevents == NULL || dprevents == NULL || dp_rfdidx == NULL )
	return -1;
    (void) mm_memset( dp_rfdidx, 0, sizeof(int) * nf );
    return 0;
    }


static void
devpoll_add_fd( int fd, int rw )
    {
    if ( ndpevents >= maxdpevents )
	{
	syslog( LOG_ERR, "too many fds in devpoll_add_fd!" );
	return;
	}
    dpevents[ndpevents].fd = fd;
    switch ( rw )
	{
	case FDW_READ: dpevents[ndpevents].events = POLLIN; break;
	case FDW_WRITE: dpevents[ndpevents].events = POLLOUT; break;
	default: break;
	}
    ++ndpevents;
    }


static void
devpoll_del_fd( int fd )
    {
    if ( ndpevents >= maxdpevents )
	{
	syslog( LOG_ERR, "too many fds in devpoll_del_fd!" );
	return;
	}
    dpevents[ndpevents].fd = fd;
    dpevents[ndpevents].events = POLLREMOVE;
    ++ndpevents;
    }


static int
devpoll_watch( long timeout_msecs )
    {
    int i, r;
    struct dvpoll dvp;

    r = sizeof(struct pollfd) * ndpevents;
    if ( r > 0 && write( dp, dpevents, r ) != r )
	return -1;

    ndpevents = 0;
    dvp.dp_fds = dprevents;
    dvp.dp_nfds = nfiles;
    dvp.dp_timeout = (int) timeout_msecs;

    r = ioctl( dp, DP_POLL, &dvp );
    if ( r == -1 )
	return -1;

    for ( i = 0; i < r; ++i )
	dp_rfdidx[dprevents[i].fd] = i;

    return r;
    }


static int
devpoll_check_fd( int fd )
    {
    int ridx = dp_rfdidx[fd];

    if ( ridx < 0 || ridx >= nfiles )
	{
	syslog( LOG_ERR, "bad ridx (%d) in devpoll_check_fd!", ridx );
	return 0;
	}
    if ( ridx >= nreturned )
	return 0;
    if ( dprevents[ridx].fd != fd )
	return 0;
    if ( dprevents[ridx].revents & POLLERR )
	return 0;
    switch ( fd_rw[fd] )
	{
	case FDW_READ: return dprevents[ridx].revents & ( POLLIN | POLLHUP | POLLNVAL );
	case FDW_WRITE: return dprevents[ridx].revents & ( POLLOUT | POLLHUP | POLLNVAL );
	default: return 0;
	}
    }


static int
devpoll_get_fd( int ridx )
    {
    if ( ridx < 0 || ridx >= nfiles )
	{
	syslog( LOG_ERR, "bad ridx (%d) in devpoll_get_fd!", ridx );
	return -1;
	}
    return dprevents[ridx].fd;
    }


# else /* HAVE_DEVPOLL */


#  ifdef HAVE_POLL

static mm_array_ptr<struct pollfd> pollfds = NULL;
static int npoll_fds;
static mm_array_ptr<int> poll_fdidx = NULL;
static mm_array_ptr<int> poll_rfdidx = NULL;


static int
poll_init( int nf )
    {
    int i;

    pollfds = MM_ARRAY_ALLOC(struct pollfd, nf );
    poll_fdidx = MM_ARRAY_ALLOC( int, nf );
    poll_rfdidx = MM_ARRAY_ALLOC( int, nf );
    if ( pollfds == NULL || poll_fdidx == NULL || poll_rfdidx == NULL )
	return -1;
    for ( i = 0; i < nf; ++i )
	pollfds[i].fd = poll_fdidx[i] = -1;
    return 0;
    }


static void
poll_add_fd( int fd, int rw )
    {
    if ( npoll_fds >= nfiles )
	{
	syslog( LOG_ERR, "too many fds in poll_add_fd!" );
	return;
	}
    pollfds[npoll_fds].fd = fd;
    switch ( rw )
	{
	case FDW_READ: pollfds[npoll_fds].events = POLLIN; break;
	case FDW_WRITE: pollfds[npoll_fds].events = POLLOUT; break;
	default: break;
	}
    poll_fdidx[fd] = npoll_fds;
    ++npoll_fds;
    }


static void
poll_del_fd( int fd )
    {
    int idx = poll_fdidx[fd];

    if ( idx < 0 || idx >= nfiles )
	{
	syslog( LOG_ERR, "bad idx (%d) in poll_del_fd!", idx );
	return;
	}
    --npoll_fds;
    pollfds[idx] = pollfds[npoll_fds];
    poll_fdidx[pollfds[idx].fd] = idx;
    pollfds[npoll_fds].fd = -1;
    poll_fdidx[fd] = -1;
    }


static int
poll_watch( long timeout_msecs )
    {
    int r, ridx, i;

    r = poll( _GETPTR(struct pollfd, pollfds), npoll_fds, (int) timeout_msecs );
    if ( r <= 0 )
	return r;

    ridx = 0;
    for ( i = 0; i < npoll_fds; ++i )
	if ( pollfds[i].revents &
	     ( POLLIN | POLLOUT | POLLERR | POLLHUP | POLLNVAL ) )
	    {
	    poll_rfdidx[ridx++] = pollfds[i].fd;
	    if ( ridx == r )
		break;
	    }

    return ridx;	/* should be equal to r */
    }


static int
poll_check_fd( int fd )
    {
    int fdidx = poll_fdidx[fd];

    if ( fdidx < 0 || fdidx >= nfiles )
	{
	syslog( LOG_ERR, "bad fdidx (%d) in poll_check_fd!", fdidx );
	return 0;
	}
    if ( pollfds[fdidx].revents & POLLERR )
	return 0;
    switch (fd_rw[fd] )
	{
	case FDW_READ: return pollfds[fdidx].revents & ( POLLIN | POLLHUP | POLLNVAL );
	case FDW_WRITE: return pollfds[fdidx].revents & ( POLLOUT | POLLHUP | POLLNVAL );
	default: return 0;
	}
    }


static int
poll_get_fd( int ridx )
    {
    if ( ridx < 0 || ridx >= nfiles )
	{
	syslog( LOG_ERR, "bad ridx (%d) in poll_get_fd!", ridx );
	return -1;
	}
    return poll_rfdidx[ridx];
    }

#  else /* HAVE_POLL */


#   ifdef HAVE_SELECT

static fd_set master_rfdset;
static fd_set master_wfdset;
static fd_set working_rfdset;
static fd_set working_wfdset;
static mm_array_ptr<int> select_fds = NULL;
static mm_array_ptr<int> select_fdidx = NULL;
static mm_array_ptr<int> select_rfdidx = NULL;
static int nselect_fds;
static int maxfd;
static int maxfd_changed;


static int
select_init( int nf )
    {
    int i;

    FD_ZERO( &master_rfdset );
    FD_ZERO( &master_wfdset );
    select_fds = MM_ARRAY_ALLOC( int, nf );
    select_fdidx = MM_ARRAY_ALLOC(int, nf );
    select_rfdidx = MM_ARRAY_ALLOC( int, nf );
    if ( select_fds == NULL || select_fdidx == NULL ||
	 select_rfdidx == NULL )
	return -1;
    nselect_fds = 0;
    maxfd = -1;
    maxfd_changed = 0;
    for ( i = 0; i < nf; ++i )
	select_fds[i] = select_fdidx[i] = -1;
    return 0;
    }


static void
select_add_fd( int fd, int rw )
    {
    if ( nselect_fds >= nfiles )
	{
	syslog( LOG_ERR, "too many fds in select_add_fd!" );
	return;
	}
    select_fds[nselect_fds] = fd;
    switch ( rw )
	{
	case FDW_READ: FD_SET( fd, &master_rfdset ); break;
	case FDW_WRITE: FD_SET( fd, &master_wfdset ); break;
	default: break;
	}
    if ( fd > maxfd )
	maxfd = fd;
    select_fdidx[fd] = nselect_fds;
    ++nselect_fds;
    }


static void
select_del_fd( int fd )
    {
    int idx = select_fdidx[fd];

    if ( idx < 0 || idx >= nfiles )
	{
	syslog( LOG_ERR, "bad idx (%d) in select_del_fd!", idx );
	return;
	}

    --nselect_fds;
    select_fds[idx] = select_fds[nselect_fds];
    select_fdidx[select_fds[idx]] = idx;
    select_fds[nselect_fds] = -1;
    select_fdidx[fd] = -1;

    FD_CLR( fd, &master_rfdset );
    FD_CLR( fd, &master_wfdset );

    if ( fd >= maxfd )
	maxfd_changed = 1;
    }


static int
select_get_maxfd( void )
    {
    if ( maxfd_changed )
	{
	int i;
	maxfd = -1;
	for ( i = 0; i < nselect_fds; ++i )
	    if ( select_fds[i] > maxfd )
		maxfd = select_fds[i];
	maxfd_changed = 0;
	}
    return maxfd;
    }


static int
select_watch( long timeout_msecs )
    {
    int mfd;
    int r, idx, ridx;

    working_rfdset = master_rfdset;
    working_wfdset = master_wfdset;
    mfd = select_get_maxfd();
    if ( timeout_msecs == INFTIM )
       r = select(
           mfd + 1, &working_rfdset, &working_wfdset, (fd_set*) 0,
           (struct timeval*) 0 );
    else
	{
	struct timeval timeout;
	timeout.tv_sec = timeout_msecs / 1000L;
	timeout.tv_usec = ( timeout_msecs % 1000L ) * 1000L;
	r = select(
	   mfd + 1, &working_rfdset, &working_wfdset, (fd_set*) 0, &timeout );
	}
    if ( r <= 0 )
	return r;

    ridx = 0;
    for ( idx = 0; idx < nselect_fds; ++idx )
	if ( select_check_fd( select_fds[idx] ) )
	    {
	    select_rfdidx[ridx++] = select_fds[idx];
	    if ( ridx == r )
		break;
	    }

    return ridx;	/* should be equal to r */
    }


static int
select_check_fd( int fd ) {
    switch ( fd_rw[fd] )
	{
	case FDW_READ: return FD_ISSET( fd, &working_rfdset );
	case FDW_WRITE: return FD_ISSET( fd, &working_wfdset );
	default: return 0;
	}
    }


static int
select_get_fd( int ridx )
    {
    if ( ridx < 0 || ridx >= nfiles )
	{
	syslog( LOG_ERR, "bad ridx (%d) in select_get_fd!", ridx );
	return -1;
	}
    return select_rfdidx[ridx];
    }

#   endif /* HAVE_SELECT */

#  endif /* HAVE_POLL */

# endif /* HAVE_DEVPOLL */

#endif /* HAVE_KQUEUE */
