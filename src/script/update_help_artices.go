package main

import (
	"fmt"
	"io"
	"net/http"
	"os"
	"path/filepath"
)

const url = "https://support.toggl.com/sitemap/"

func main() {
	response, err := http.Get(url)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	defer response.Body.Close()

	fileName := filepath.Join("src", "help", "sitemap.json")

	os.Remove(fileName)

	output, err := os.Create(fileName)
	if err != nil {
		fmt.Println("Error while creating", fileName, "-", err)
		return
	}
	defer output.Close()

	n, err := io.Copy(output, response.Body)
	if err != nil {
		fmt.Println("Error while downloading", url, "-", err)
		return
	}

	fmt.Println(n, "bytes downloaded.")
}
