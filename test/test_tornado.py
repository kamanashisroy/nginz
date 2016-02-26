import tornado.ioloop
import tornado.iostream
import socket

class NginZChatClient:
	def connect(self, index):
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
		self.stream = tornado.iostream.IOStream(s)
		#stream.on_connection_close(self.on_close);
		self.stream.connect(("localhost", 9399), self.wait_for_prompt)
		self.index = index

	def wait_for_prompt(self):
		self.stream.read_until(b"name?\n", self.send_login)

	def send_login(self, data):
		#print data
		self.stream.write(b"name" + `self.index` + "\n")
		self.stream.read_until(b"!\n", self.on_login)

	def on_login(self, data):
		#print data
		if index < 500:
			self.stream.write(b"/join ONE\n")
		else:
			self.stream.write(b"/join THREE\n")
		self.stream.read_until(b"end of list\n", self.talk)

	def talk(self, data):
		self.stream.write(b"great\n")
		try:
			self.stream.write(b"/quit\n")
		except tornado.iostream.StreamClosedError:
			print "closed"
		else:
			self.stream.close()

	def on_close(self):
		print "closed"


if __name__ == '__main__':
	for index in range(1, 1000):
		client = NginZChatClient()
		client.connect(index);

	tornado.ioloop.IOLoop.current().start()
	# tornado.ioloop.IOLoop.current().stop()

