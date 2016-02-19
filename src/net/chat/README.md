
Chat module has the following constructs.

- `chat\_connection` : It holds the connection data. It uses state-pattern to handle user data. The `on\_response\_callback` handles login and broadcast action. The `send\_content` allows proxy-pattern(for example, tunneling over HTTP) while sending data.
- `chat\_hooks` : It is the API to create and execture command and requests on `chat\_connection`. It is used by the application(for example, web application) to handle chat data.

