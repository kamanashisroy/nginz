echo "softquitall\n" | socat - UNIX-CLIENT:/tmp/nginz.sock
echo "quitall\n" | socat - UNIX-CLIENT:/tmp/nginz.sock
