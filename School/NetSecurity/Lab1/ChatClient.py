import socket
import select
import errno

from Crypto.Cipher import AES
from Crypto.Util.Padding import pad, unpad

key = b'EE82E70D5C71E0D0C79F545FC85506E2'
cipher = AES.new(key, AES.MODE_ECB)

HEADER_LENGTH = 10
BLK_SIZE = 16

IP = "127.0.0.1"
PORT = 25791
my_username = input("Username: ")

# Create a socket
# socket.AF_INET - address family, IPv4, some otehr possible are AF_INET6, AF_BLUETOOTH, AF_UNIX
# socket.SOCK_STREAM - TCP, conection-based, socket.SOCK_DGRAM - UDP, connectionless, datagrams, socket.SOCK_RAW - raw IP packets
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect to a given ip and port
client_socket.connect((IP, PORT))

# Set connection to non-blocking state, so .recv() call won;t block, just return some exception we'll handle
client_socket.setblocking(False)

# Prepare username and header and send them
# We need to encode username to bytes, then count number of bytes and prepare header of fixed size, that we encode to bytes as well
username = my_username.encode('utf-8')
username_ciphertext = cipher.encrypt(pad(username,16))
username_header = f"{len(username_ciphertext):<{HEADER_LENGTH}}".encode('utf-8')
username_header_ciphertext = cipher.encrypt(pad(username_header,16))
client_socket.send(username_header_ciphertext)
client_socket.send(username_ciphertext)

while True:

    # Wait for user to input a message
    message = input(f'{my_username} > ')

    # If message is not empty - send it
    if message:

        # Encode message to bytes, prepare header and convert to bytes, like for username above, then send
        message = message.encode('utf-8')
        message_ct = cipher.encrypt(pad(message,16))
        message_header = f"{len(message_ct):<{HEADER_LENGTH}}".encode('utf-8')
        message_header_ct = cipher.encrypt(pad(message_header,16))
        client_socket.send(message_header_ct + message_ct)

    try:
        # Now we want to loop over received messages (there might be more than one) and print them
        while True:

            # Receive our "header" containing username length, it's size is defined and constant
            username_header_ciphertext = client_socket.recv(BLK_SIZE)
            username_header = unpad(cipher.decrypt(username_header_ciphertext),16)

            # If we received no data, server gracefully closed a connection, for example using socket.close() or socket.shutdown(socket.SHUT_RDWR)
            if not len(username_header):
                print('Connection closed by the server')
                sys.exit()

            # Convert header to int value
            username_length = int(username_header.decode('utf-8').strip())

            # Receive and decode username
            username_ct = client_socket.recv(username_length)
            username = unpad(cipher.decrypt(username_ct),16).decode('utf-8')

            # Now do the same for message (as we received username, we received whole message, there's no need to check if it has any length)
            message_header_ct = client_socket.recv(BLK_SIZE)
            message_header = unpad(cipher.decrypt(message_header_ct),16)
            message_length = int(message_header.decode('utf-8').strip())
            message_ct = client_socket.recv(message_length)
            message =  unpad(cipher.decrypt(message_ct),16).decode('utf-8')

            # Print message
            print(f'{username} > {message}')

    except IOError as e:
        # This is normal on non blocking connections - when there are no incoming data error is going to be raised
        # Some operating systems will indicate that using AGAIN, and some using WOULDBLOCK error code
        # We are going to check for both - if one of them - that's expected, means no incoming data, continue as normal
        # If we got different error code - something happened
        if e.errno != errno.EAGAIN and e.errno != errno.EWOULDBLOCK:
            print('Reading error: {}'.format(str(e)))
            sys.exit()

        # We just did not receive anything
        continue

    except Exception as e:
        # Any other exception - something happened, exit
        print('Reading error: '.format(str(e)))
        sys.exit()
