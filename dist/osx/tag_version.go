package main

import (
	"bytes"
	"flag"
	"fmt"
	"os"
	"os/exec"
	"strconv"
	"strings"
)

var verbose = flag.Bool("verbose", false, "verbose")
var bumpminor = flag.Bool("bumpminor", false, "bumpminor")
var predict = flag.Bool("predict", false, "predict")
var buffer bytes.Buffer

func main() {
	flag.Parse()

	if *verbose {
		buffer.WriteString("Fetch tags\n")
	}
	b, err := exec.Command("git", "fetch", "--tags", "--force").CombinedOutput()
	if err != nil {
		fmt.Println(buffer.String())
		fmt.Println(string(b))
		fmt.Println(err.Error())
		os.Exit(1)
	}
	if *verbose {
		fmt.Println(string(b))
	}

	if *verbose {
		buffer.WriteString("Get current head\n")
	}
	b, err = exec.Command("git", "rev-parse", "HEAD").CombinedOutput()
	if err != nil {
		fmt.Println(buffer.String())
		fmt.Println(string(b))
		fmt.Println(err.Error())
		os.Exit(2)
	}
	if *verbose {
		fmt.Println(string(b))
	}
	head := strings.Trim(string(b), "\n")
	if *verbose {
		buffer.WriteString("HEAD is ")
		buffer.WriteString(head)
		buffer.WriteString("\n")
	}

	if *verbose {
		buffer.WriteString("Assign version number\n")
	}
	var version string
	b, err = exec.Command("git", "tag", "--points-at", head).CombinedOutput()
	if err == nil && len(strings.Trim(string(b), "\n")) > 0 {
		version = strings.Trim(string(b), "\n")
		if *verbose {
			buffer.WriteString("HEAD is already tagged, version number")
			buffer.WriteString(version)
			buffer.WriteString("\n")
		}
		fmt.Println(version[1:])
		os.Exit(0)
	}

	if *verbose {
		buffer.WriteString("HEAD is not tagged, need to create new version number\n")
		buffer.WriteString("Get previous tag\n")
	}
	b, err = exec.Command("git", "tag", "-l", "v7.*.*", "--sort=-v:refname").CombinedOutput()
	if err != nil {
		fmt.Println(buffer.String())
		fmt.Println(string(b))
		fmt.Println(err.Error())
		os.Exit(3)
	}
	previous := strings.Split(string(b), "\n")[0]
	if *verbose {
		buffer.WriteString("Previous tag: ")
		buffer.WriteString(previous)
		buffer.WriteString("\n")
	}

	if *verbose {
		buffer.WriteString("Convert to major.minor.build version format\n")
	}
	parts := strings.Split(previous, ".")
	if len(parts) == 2 {
		var newparts []string
		newparts = append(newparts, parts[0])
		newparts = append(newparts, "0")
		newparts = append(newparts, parts[1])
		parts = newparts
	}
	if len(parts) != 3 {
		panic(fmt.Errorf("Unexpected number of version parts: 3 expected, got %d  [%s]", len(parts), previous))
	}
	if *verbose {
		buffer.WriteString("Previous version was")
		buffer.WriteString(strings.Join(parts, "."))
		buffer.WriteString("\n")
	}

	if *bumpminor {
		if *verbose {
			buffer.WriteString("Bumping minor version number\n")
		}
		minor, err := strconv.ParseInt(parts[1], 10, 64)
		if err != nil {
			fmt.Println(buffer.String())
			fmt.Println(err.Error())
			os.Exit(1)
		}
		parts[1] = fmt.Sprintf("%d", minor+1)
		parts[2] = "0"
	} else {
		if *verbose {
			buffer.WriteString("Increment patch number\n")
		}
		patch, err := strconv.ParseInt(parts[2], 10, 64)
		if err != nil {
			fmt.Println(buffer.String())
			fmt.Println(err.Error())
			os.Exit(4)
		}
		parts[2] = fmt.Sprintf("%d", patch+1)
	}
	version = strings.Join(parts, ".")
	if *verbose {
		buffer.WriteString("Assigned new version number")
		buffer.WriteString(version)
		buffer.WriteString("to HEAD")
		buffer.WriteString(head)
		buffer.WriteString("\n")
	}

	if !*predict {
		if *verbose {
			buffer.WriteString("Create new tag\n")
		}
		b, err = exec.Command("git", "tag", version, "-m", version).CombinedOutput()
		if err != nil {
			fmt.Println(buffer.String())
			fmt.Println(string(b))
			fmt.Println(err.Error())
			os.Exit(5)
		}

		if *verbose {
			buffer.WriteString("Push tags\n")
		}
		b, err = exec.Command("git", "push", "--tags", "--verbose").CombinedOutput()
		if err != nil {
			fmt.Println(buffer.String())
			fmt.Println(string(b))
			fmt.Println(err.Error())
			os.Exit(6)
		}
	}

	fmt.Println(version[1:])
	os.Exit(0)
}
