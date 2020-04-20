# A list of benchmark candidates to be rewritten using checked c

Note: The numbers in the parentheses are estimations of SLOC for c code.

## Security Critical Program
- sudo([90k](https://www.openhub.net/p/sudo))
- passwd([1.3k](https://www.openhub.net/p/passwd))
- sshd
- thttpd
- Apache/Tomcat/Nginx/Squid
- dhcpd
- xinetd
- Bonjour
- OpenSSL([350k](https://www.openhub.net/p/openssl))
- vsftpd
- Any setuid root program

## Performance Program
- SPEC
- bzip/gzip/openssl/gpg/tar
- MiBench
- MultiSource/Application in LLVM test suite

## Web Browser
- Rendering engine
- Javascript engine