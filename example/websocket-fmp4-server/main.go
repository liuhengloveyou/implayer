package main

import (
	"flag"
	"fmt"
	"net/http"
	"os"

	"golang.org/x/net/websocket"
)

var path = flag.String("i", "", "視頻文件")
var buf []byte

func WebsocketServer(ws *websocket.Conn) {
	fmt.Println("WebsocketServer: ", ws.RemoteAddr(), ws.PayloadType)

	var req []byte
	n, err := ws.Read(req)

	i := 0

	for {
		websocket.Message.Send(ws, buf[i:i+1024])
		fmt.Println("ws: ", i, len(buf))

		i = i + 1024
		if i > len(buf)-1024 {
			websocket.Message.Send(ws, buf[i:])
			break
		}

		// time.Sleep(time.Second)
	}

	// ws.Write(buf)
}

func main() {
	flag.Parse()

	if path == nil {
		fmt.Println("file?")
		return
	}

	fmt.Println(">>>", *path)

	var err error
	buf, err = os.ReadFile(*path)
	if err != nil {
		panic(err)
	}

	http.Handle("/ws", websocket.Handler(WebsocketServer))
	err = http.ListenAndServe(":8080", nil)
	if err != nil {
		panic("ListenAndServe: " + err.Error())
	}

	return
}
