import socket.timeout as TimeoutException
# set timeout 5 second
clientsocket.settimeout(5)
for i in range(0,10):
  sequence_number = i
  start = time.time()
  clientSocket.sendto("Ping " + str(i) + " " + str(start), server)
  # Receive the client packet along with the address it is coming from
  try:
    message, address = clientSocket.recvfrom(1024)
  except TimeoutException:
    print("Timeout!!! Try again...")
    continue
  end = time.time()
  if message != '':
    print message
    rtt = end - start
    print "RTT = " + str(rtt)
