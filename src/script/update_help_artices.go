package main

import "net/http"
import "fmt"
import "os"

func main() {
	response, err := http.Get("https://support.toggl.com/sitemap/")
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	defer response.Body.Close()
}
