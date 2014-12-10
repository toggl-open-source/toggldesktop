package main

import (
	"flag"
	"os/exec"
	"strings"
	"os"
	"fmt"
)

var executable = flag.String("executable", "", "name of the app executable")
var destination = flag.String("destination", "", "where to put the executable deps")

var knownDeps = []string{
	"libQt",
	"libPoco",
	"libbugsnag",
	"libTogglDesktop",
	"libicu",
	"libjson",
	"libssl",
	"libcrypto",
}

func main() {
	flag.Parse()

	if len(*executable) == 0 {
		fmt.Println("executable parameter is required")
		os.Exit(1)
	}
	if len(*destination) == 0 {
		fmt.Println("destination parameter is required")
		os.Exit(1)
	}

	b, err := exec.Command("ldd", *executable).CombinedOutput()
	if err != nil {
		fmt.Println(err, string(b))
		os.Exit(1)
	}

	deps := strings.Split(string(b), "\n")

	fmt.Println(len(deps), "dependencies found")

	for _, s := range deps {
		if !strings.Contains(s, "=>") {
			continue
		}
		name := strings.Split(s, "=> ")[1]
		name = strings.Split(name, " (")[0]
		if len(name) == 0 {
			continue
		}

		if name == "not found" {
			fmt.Println(s)
			fmt.Println("Library dependency not found")
			os.Exit(1)
		}
		
		required := false
		for _, depName := range knownDeps {
			if strings.Contains(name, depName) {
				required = true
				break
			}
		}
		if !required {
			continue
		}
		_, err := exec.Command("cp", name, *destination).CombinedOutput()
		if err != nil {
			fmt.Println(err, string(b))
			fmt.Println("cp subcommand failed")
			fmt.Println("cp", name, *destination)
			os.Exit(1)
		}
	}

	fmt.Println(len(deps), "dependencies copied")

	os.Exit(0)
}
