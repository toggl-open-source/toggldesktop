package main

import (
	"io/ioutil"
	"os"
	"strings"
)

func main() {
	if err := findMissingInterfaces(); err != nil {
		println(err.Error())
		os.Exit(1)
	}
	os.Exit(0)
}

func findMissingInterfaces() error {
	// read in API declarations
	b, err := ioutil.ReadFile("src/toggl_api.h")
	if err != nil {
		return err
	}
	// look for funcs in the source
	l := strings.Split(string(b), "\n")
	var funcs []string
	for _, s := range l {
		// accept only exported funcs
		if !strings.Contains(s, "TOGGL_EXPORT") {
			continue
		}
		// accept only funcs
		if !strings.Contains(s, "toggl_") {
			continue
		}
		// skip callbacks
		if strings.Contains(s, "toggl_on_") {
			continue
		}
		// get func name without prefix
		parts := strings.Split(s, "toggl_")
		if len(parts) < 2 {
			continue
		}
		name := strings.Replace(parts[1], "(", "", -1)
		funcs = append(funcs, name)
	}
	for _, s := range funcs {
		println(s)
	}
	return nil
}
