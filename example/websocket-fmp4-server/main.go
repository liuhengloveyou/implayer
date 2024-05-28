package main

import (
	"flag"
	"net/http"
	"os"

	"golang.org/x/net/websocket"
)

var path = flag.String("i", "", "視頻文件")

func WebsocketServer(ws *websocket.Conn) {
	buf, err := os.ReadFile(*path)
	if err != nil {
		panic(err)
	}

	ws.Write(buf)
}

func main() {
	http.Handle("/ws", websocket.Handler(WebsocketServer))

	err := http.ListenAndServe(":8080", nil)
	if err != nil {
		panic("ListenAndServe: " + err.Error())
	}

	return
}
