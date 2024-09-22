const WebSocket = require('ws');

class Room{
    id;
    l_idx;
    r_idx;

    constructor(id, l_idx, r_idx){
        this.id = id;
        this.l_idx = l_idx;
        this.r_idx = r_idx;
    }

    get id(){
        return this.id;
    }

    get left(){
        return this.l_idx;
    }

    get right(){
        return this.r_idx;
    }
}

class Client{
    static Type = Object.freeze({
        Waiting: Symbol("waiting"),
        Playing: Symbol("playing")
    });

    socket;
    id;
    type;
    room_id;

    constructor(socket, id){
        this.id = id;
        this.socket  = socket;
        this.type = Client.Waiting;
        this.room_id = -1;
    }

    get type(){
        return this.type;
    }

    set type(_type){
        this.type = _type;
    }

    get id(){
        return this.id;
    }

    get socket(){
        return this.socket;
    }
}

class WSServer{
    ws_server;
    port;
    rooms;
    clients;


    uid(){
        return (new Date()).getTime();
    }

    static log(msg){
        console.log(`[WSServer] ${msg}`);
    }

    constructor(PORT = 3000){
        this.port = PORT;
        this.rooms = Array();
        this.clients = Array();
    }

    listen(){
        this.ws_server = new WebSocket.Server({
            port: this.port
        });

        this.ws_server.on('connection', this.on_connection.bind(this));
        this.ws_server.on('close', this.on_close);
        this.ws_server.on('error', this.on_error);
        this.ws_server.on('listening', this.on_listening);
        this.ws_server.on('headers', this.on_headers);
        
        WSServer.log(`Listening PORT: ${this.port}`);
    }

    on_connection(socket, req){
        WSServer.log(`New Connection: ${req.headers['x-forwarded-for'] || req.socket.remoteAddress}`);
        
        this.confirm_connection(socket);
        this.confirm_room();
    }
    on_close(socket){
        console.log(`Close socket: ${socket}`);
    }
    on_error(){

    }
    on_listening(){

    }
    on_headers(){

    }

    on_message(msg){
        var data = JSON.parse(msg);

        if(data["type"] == "update"){
            this.rooms.forEach(room => {
                if(room.id == data["room_id"]){
                    this.clients
                        .find(client => client.id == (room.left == data["client_id"] ? room.right : room.left))
                        .socket.send(msg)
                    
                    return;
                }
            })
        }
    }

    generate_room(l_idx, r_idx){
        this.clients[l_idx].type = Client.Type.Playing;
        this.clients[r_idx].type = Client.Type.Playing;

        this.rooms.push(new Room(
                this.uid(), 
                this.clients[l_idx].id, 
                this.clients[r_idx].id
            ));
        
        const notificate = (socket) => {
            socket.send(JSON.stringify({
                "type" : "set_room",
                "id" : this.rooms[this.rooms.length - 1].id,
                "init" : {
                    "left" : this.clients[l_idx].id,
                    "right" : this.clients[r_idx].id
                }
            }));
        }

        notificate(this.clients[l_idx].socket);
        notificate(this.clients[r_idx].socket);

        WSServer.log(`New Room: ${this.clients[l_idx].id} | ${this.clients[r_idx].id}`);
    }

    confirm_room(){
        for(let i = 0; i < this.clients.length; i++){
            if(this.clients[i].type == Client.Type.Playing) continue;

            let j = i + 1;
            for(let j = i + 1; j < this.clients.length; j++){
                if(this.clients[j].type == Client.Type.Waiting) break;
            }

            if(j < this.clients.length){
                this.generate_room(i, j);
            }
        }

    }

    confirm_connection(socket){
        this.clients.push(new Client(socket,this.uid()));
        
        socket.send(JSON.stringify({
            "type" : "set_id",
            "id" : this.clients[this.clients.length - 1].id
        }));
        
        socket.on('error', ()=>{console.log("123")});
        socket.on('message', this.on_message.bind(this));
    }
}

const wsServer = new WSServer();
wsServer.listen();
