// index.js
// bringing in express to help me build this simple web server
const express = require('express')
// using port 3004 since other ports are already taken by previous questions
const port = 3004
// creating my express application instance that'll handle all the requests
app = express();
// telling express to serve up everything in the public folder so my HTML and assets are accessible
app.use(express.static('public'));

// starting up the server and listening for incoming connections on my chosen port
// listening on 0.0.0.0 to accept connections from any network interface
app.listen(port, '0.0.0.0', () => {
  // logging a friendly message so I know the server is running and where to find it
  console.log(`Light level visualizer app listening at http://localhost:${port}/index.html`)
  console.log(`Also accessible on your network at http://172.26.30.167:${port}/index.html`)
})
