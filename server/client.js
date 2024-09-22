const WebSocket = require('ws');

const server = new WebSocket("wss://second-quick-armadillo.glitch.me",{
    headers:{
        "user-agent" : "ws"
    }
})

server.on('open', () => {
    server.send("Hello");
})

server.on('message', msg => {
    console.log(msg);
})