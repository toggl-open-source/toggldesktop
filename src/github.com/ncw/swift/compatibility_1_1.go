// Go 1.1 and later compatibility functions
//
// +build go1.1

package swift

import (
	"net/http"
	"time"
)

// Cancel the request
func cancelRequest(tr *http.Transport, req *http.Request) {
	tr.CancelRequest(req)
}

// Reset a timer
func resetTimer(t *time.Timer, d time.Duration) {
	t.Reset(d)
}
