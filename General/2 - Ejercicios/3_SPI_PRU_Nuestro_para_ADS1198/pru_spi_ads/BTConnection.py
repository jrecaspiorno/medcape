import bluetooth
import time
import sys

print "Starting programm"


#
# Bluetooth connection
#

# Open Bluetooth socket and listen for connections
serverSocket = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
serverSocket.bind(("", bluetooth.PORT_ANY))
serverSocket.listen(1)  # Listen for any incoming connections
serverSocketChannel = serverSocket.getsockname()[1]

# Random UUID (with medical devices there is a kind of standard)
uuid = "34B1CF4D-1069-4AD6-89B6-E161D79BE4D8"

# Advertise our Bluetooth service so other devices can find it
bluetooth.advertise_service(serverSocket, "BeagleBoneService",
        service_id = uuid,
        service_classes = [uuid, bluetooth.SERIAL_PORT_CLASS],
        profiles = [bluetooth.SERIAL_PORT_PROFILE])


#
# Pipe
#

#pipeName = "fifo.tmp"
pipeName = "ztest.data"
#outputFileName = "data.dat"

#
# The tag has a 1 byte length (TAG = 0xB5)
# The ADC samples N-channels and each channel stores M-samples so
# 	the size of the packet is N*M, but we wrap half the buffer so N*M/2,
# 	each sample is uint16
# The Nonin packet contains 6 bytes: 2 (Pleth) + 1 (SpO2) + 2 (Pleth) + 1 (SpO2)
#
TAG_SIZE          = 1
ADC_CHANNELS      = 2
ADC_SAMP_CHANNEL  = 12
ADC_PACKET_SIZE   = 2 * (ADC_CHANNELS * ADC_SAMP_CHANNEL) / 2.0
NONIN_PACKET_SIZE = 6

chunkSize = int(TAG_SIZE + ADC_PACKET_SIZE + NONIN_PACKET_SIZE)

#
# Combining Bluetooth module with the pipe
#

while True:

	# Wait for an incoming connection
	print "Waiting for a new connection on channel", serverSocketChannel
	clientSocket, clientAddress = serverSocket.accept()
	print "Accepted connection from ", clientAddress
	time.sleep(1)
	
	# Send the data received from pipe to connected device
	try:
	
		pipeFile = open(pipeName, "rb")
		print pipeName, "opened"
		#outputFile = open(outputFileName, "wb")
		#print outputFileName, "opened"

		while True:
			try:
				while True:
					chunk = pipeFile.read(chunkSize)
					#print len(chunk)
					if len(chunk) <= 0:
						break
					else:
						clientSocket.send(chunk)
						#data = clientSocket.recv(1024)
						#print "Received:", chunk
						#sys.stdout.flush()

			except:
				break  # Device is disconnected
				
	except IOError:
		print "Pipe cannot be established"
	
	print "Device disconnected"
	clientSocket.close()


