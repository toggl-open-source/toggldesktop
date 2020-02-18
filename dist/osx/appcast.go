package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"
)

const filename = "src/branding/appcast_template.xml"

var version = flag.String("version", "", "the app version to set in the plist file")
var date = flag.String("date", "", "publish date")
var appURL = flag.String("appUrl", "", "url of the application package")
var filesize = flag.String("filesize", "0", "filesize")
var signature = flag.String("signature", "00", "signature hash")
var verbose = flag.Bool("verbose", false, "log out processing parameters")
var platform = flag.String("platform", "", "windows, linux or darwin")

func main() {
	flag.Parse()

	b, err := ioutil.ReadFile(filename)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	appcast, err := processAppCast(string(b))
	if err != nil {
		fmt.Println(err)
		os.Exit(0)
	}
	if err := ioutil.WriteFile(filepath.Join("tmp", *platform+"_dev_appcast.xml"), []byte(appcast.Content), 0644); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

// AppCast represents the Sparkle update info in XML format
type AppCast struct {
	Content string
}

func processAppCast(contents string) (*AppCast, error) {

	if *verbose {
		fmt.Println("--------------------------")
		fmt.Println("Processing appcast")
		fmt.Println("--------------------------")
		fmt.Println("version:", *version)
		fmt.Println("date:", *date)
		fmt.Println("appUrl:", *appURL)
		fmt.Println("appCastUrl:", *platform+"_dev_appcast.xml")
		fmt.Println("filesize:", *filesize)
		fmt.Println("signature:", *signature)
		fmt.Println("--------------------------")
	}

	var appcast AppCast

	contents = strings.Replace(contents, "{VERSION}", *version, -1)
	contents = strings.Replace(contents, "{DATE}", *date, -1)
	contents = strings.Replace(contents, "{APPCAST_URL}", *platform+"_dev_appcast.xml", -1)
	contents = strings.Replace(contents, "{APP_URL}", *appURL, -1)
	contents = strings.Replace(contents, "{LENGTH}", *filesize, -1)
	contents = strings.Replace(contents, "{SIGNATURE}", *signature, -1)

	appcast.Content = contents
	return &appcast, nil
}
