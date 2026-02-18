package main
import (
	"bufio"
	"fmt"
	"net"
	"os"
	"strings"
)
const (
	MyPort     = 8080
	TargetPort = 8080
	ColReset   = "\033[0m"
	ColGreen   = "\033[32m"
	ColCyan    = "\033[36m"
	ClearLine  = "\033[2K\r"
)

func main() {
	if len(os.Args) != 2 {
		fmt.Printf("Usage: go run main.go <Target IP>\n")
		os.Exit(1)
	}
	targetIP := os.Args[1]

	addr, _ := net.ResolveUDPAddr("udp", fmt.Sprintf(":%d", MyPort))
	conn, err := net.ListenUDP("udp", addr)
	if err != nil {
		fmt.Printf("Error: %v\n", err)
		os.Exit(1)
	}
	defer conn.Close()

	fmt.Printf("%s=== Go P2P Chat Started (Port %d) ===%s\n", ColCyan, MyPort, ColReset)
	fmt.Print("Input > ")

	go func() {
		buffer := make([]byte, 1024)
		for {
			n, remoteAddr, err := conn.ReadFromUDP(buffer)
			if err != nil {
				continue
			}
			msg := string(buffer[:n])
			fmt.Printf(ClearLine+"%s[%s]: %s%s\n", ColGreen, remoteAddr.IP, msg, ColReset)
			fmt.Print("Input > ")
		}
	}()

	targetAddr, _ := net.ResolveUDPAddr("udp", fmt.Sprintf("%s:%d", targetIP, TargetPort))
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		text := strings.TrimSpace(scanner.Text())
		if len(text) > 0 {
			_, err := conn.WriteToUDP([]byte(text), targetAddr)
			if err != nil {
				fmt.Printf("Send error: %v\n", err)
			}
		}
		fmt.Print("Input > ")
	}
}
