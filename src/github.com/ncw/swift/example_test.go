// Copyright...

// This example demonstrates opening a Connection and doing some basic operations.
package swift_test

import (
	"fmt"
	"github.com/ncw/swift"
)

func Example() {
	// Create a connection
	c := swift.Connection{
		UserName: "user",
		ApiKey:   "key",
		AuthUrl:  "auth_url",
	}
	// Authenticate
	err := c.Authenticate()
	if err != nil {
		panic(err)
	}
	// List all the containers
	containers, err := c.ContainerNames(nil)
	fmt.Println(containers)
	// etc...
}

var container string

func ExampleConnection_ObjectsWalk() {
	objects := make([]string, 0)
	err := c.ObjectsWalk(container, nil, func(opts *swift.ObjectsOpts) (interface{}, error) {
		newObjects, err := c.ObjectNames(container, opts)
		if err == nil {
			objects = append(objects, newObjects...)
		}
		return newObjects, err
	})
	fmt.Println("Found all the objects", objects, err)
}

func ExampleConnection_VersionContainerCreate() {
	// Use the helper method to create the current and versions container.
	if err := c.VersionContainerCreate("cds", "cd-versions"); err != nil {
		fmt.Print(err.Error())
	}
}

func ExampleConnection_VersionEnable() {
	// Build the containers manually and enable them.
	if err := c.ContainerCreate("movie-versions", nil); err != nil {
		fmt.Print(err.Error())
	}
	if err := c.ContainerCreate("movies", nil); err != nil {
		fmt.Print(err.Error())
	}
	if err := c.VersionEnable("movies", "movie-versions"); err != nil {
		fmt.Print(err.Error())
	}

	// Access the primary container as usual with ObjectCreate(), ObjectPut(), etc.
	// etc...
}

func ExampleConnection_VersionDisable() {
	// Disable versioning on a container.  Note that this does not delete the versioning container.
	c.VersionDisable("movies")
}
