package main

import (
	"bytes"
	"crypto/md5"
	"encoding/hex"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"net/http"
	"os"
	"strings"
)

const installationRegistrationToken = "NfKP7Ae9N0xa18GUzfgC5tE1G5KazxBOkWFpXtJjmhuSOKXSzymUXxoqAc3TNox"
const destination = "buildbot@35.222.140.137:/mnt/disks/assets/www_root/installers/"

// GithubRelease corresponds to Release data type in Github API
type GithubRelease struct {
	ID         int64  `json:"id,omitempty"`
	TagName    string `json:"tag_name"`
	Name       string `json:"name"`
	Prerelease bool   `json:"prerelease"`
}

// GithubReleaseAsset responds to an installer executable, under a Github release
type GithubReleaseAsset struct {
	ID                 int64  `json:"id"`
	Name               string `json:"name"`
	URL                string `json:"url"`
	BrowserDownloadURL string `json:"browser_download_url"`
}

// Env defines the configuration needed to register a release asset on Github
type Env struct {
	channel           string
	version           string
	prefixLessVersion string
	installer         string
	installerFilename string
	githubUser        string
	githubToken       string
	prerelease        bool
	platform          string
}

// TogglBackendRegistration is used to registre an installer in the Toggl backend.
// This will be replaced soon with Github-only workflow.
type TogglBackendRegistration struct {
	channel   string
	url       string
	platform  string
	version   string
	installer string
}

var registerFlag = flag.Bool("register", false, "do not release to Github; only register the installer")
var channelFlag = flag.String("channel", "dev", "do not release to Github; only register the installer to this channel")

func main() {
	flag.Parse()
	if *registerFlag {
		if err := doTogglBackendRegistration(); err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		os.Exit(0)
	}
	if err := doGithubRelease(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	os.Exit(0)
}

func isSupportedPlatform(platform string) bool {
	supportedPlatforms := []string{"windows", "darwin", "linux", "deb64"}
	for _, name := range supportedPlatforms {
		if platform == name {
			return true
		}
	}
	return false
}

func isSupportedChannel(channel string) bool {
	supportedChannels := []string{"dev", "beta", "stable"}
	for _, name := range supportedChannels {
		if channel == name {
			return true
		}
	}
	return false
}

func (env *Env) parse() error {
	env.channel = os.Getenv("CHANNEL")
	if len(env.channel) == 0 {
		env.channel = "dev"
	}
	if !isSupportedChannel(env.channel) {
		return errors.New("\033[94m[Github Upload]\033[0m - invalid CHANNEL " + env.channel)
	}

	env.version = os.Getenv("VERSION")
	if len(env.version) == 0 {
		return errors.New("\033[94m[Github Upload]\033[0m - must set VERSION to continue")
	}

	env.platform = os.Getenv("PLATFORM")
	if len(env.platform) == 0 {
		return errors.New("\033[94m[Github Upload]\033[0m - must set PLATFORM to continue")
	}

	env.prefixLessVersion = env.version
	if !strings.HasPrefix(env.version, "v") {
		env.version = "v" + env.version
	}

	env.installer = os.Getenv("INSTALLER")
	if len(env.installer) == 0 {
		return errors.New("\033[94m[Github Upload]\033[0m - must set INSTALLER to continue")
	}

	env.installerFilename = os.Getenv("INSTALLER_FILENAME")
	if len(env.installerFilename) == 0 {
		env.installerFilename = env.installer
	}

	env.githubUser = os.Getenv("GITHUB_USER")
	if len(env.githubUser) == 0 {
		return errors.New("\033[94m[Github Upload]\033[0m - Missing GITHUB_USER")
	}

	env.githubToken = os.Getenv("GITHUB_TOKEN")
	if len(env.githubToken) == 0 {
		return errors.New("\033[94m[Github Upload]\033[0m - Missing GITHUB_TOKEN")
	}

	env.prerelease = true
	isPrerelease := os.Getenv("PRERELEASE")
	if isPrerelease == "false" || isPrerelease == "0" {
		env.prerelease = false
	}

	return nil
}

func (env Env) createGithubRelease() error {
	fmt.Println("\033[94m[Github Upload]\033[0m - Creating release")

	data := GithubRelease{
		TagName:    env.version,
		Name:       env.version,
		Prerelease: env.prerelease,
	}

	b, err := json.Marshal(data)
	if err != nil {
		return err
	}

	buf := bytes.NewBuffer(b)

	req, err := http.NewRequest("POST", "https://api.github.com/repos/toggl-open-source/toggldesktop/releases", buf)
	if err != nil {
		fmt.Println("\033[94m[Github Upload]\033[0m - failed on releases POST")
		return err
	}

	client := http.Client{}

	req.SetBasicAuth(env.githubUser, env.githubToken)

	resp, err := client.Do(req)
	if err != nil {
		fmt.Println("\033[94m[Github Upload]\033[0m - failed on Client.Do")
		return err
	}
	defer resp.Body.Close()
	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		fmt.Println("\033[94m[Github Upload]\033[0m - failed on ioutil.ReadAll")
		return err
	}
	switch resp.StatusCode {
	case 422:
		fmt.Println("\033[94m[Github Upload]\033[0m - Warning! Release already exists")
		return nil
	case 200, 201:
		return nil
	}
	return fmt.Errorf("\033[94m[Github Upload]\033[0m - Installer registration failed: %s HTTP status code: %d", string(body), resp.StatusCode)
}

func (env Env) getGithubReleaseID() (int64, error) {
	client := http.Client{}
	url := fmt.Sprintf("https://api.github.com/repos/toggl-open-source/toggldesktop/releases/tags/%s", env.version)
	req, err := http.NewRequest("GET", url, nil)
	if err != nil {
		return 0, err
	}
	req.SetBasicAuth(env.githubUser, env.githubToken)

	resp, err := client.Do(req)
	if err != nil {
		return 0, err
	}
	defer resp.Body.Close()
	b, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return 0, err
	}
	switch resp.StatusCode {
	case 200:
		var release GithubRelease
		if err := json.Unmarshal(b, &release); err != nil {
			return 0, err
		}
		return release.ID, nil
	}
	return 0, fmt.Errorf("\033[94m[Github Upload]\033[0m - Failed to get release ID: %s HTTP status code: %d", string(b), resp.StatusCode)
}

func (env Env) fetchGithubAssets(releaseID int64) ([]GithubReleaseAsset, error) {
	// Fetch assets, to check if the asset already exists. If it does,
	// We'll delete it first
	url := fmt.Sprintf("https://api.github.com/repos/toggl-open-source/toggldesktop/releases/%d/assets", releaseID)
	fmt.Println("\033[94m[Github Upload]\033[0m - Fetching assets from", url)

	client := http.Client{}
	req, err := http.NewRequest("GET", url, nil)
	if err != nil {
		return nil, err
	}
	req.SetBasicAuth(env.githubUser, env.githubToken)

	resp, err := client.Do(req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	b, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return nil, err
	}
	fmt.Println(string(b))
	switch resp.StatusCode {
	case 200:
		var assets []GithubReleaseAsset
		if err := json.Unmarshal(b, &assets); err != nil {
			return nil, err
		}
		return assets, nil
	}
	return nil, fmt.Errorf("\033[94m[Github Upload]\033[0m - Failed to fetch assets ID: %s HTTP status code: %d", string(b), resp.StatusCode)
}

func (env Env) deleteGithubAsset(assetID int64) error {
	url := fmt.Sprintf("https://api.github.com/repos/toggl-open-source/toggldesktop/releases/assets/%d", assetID)
	fmt.Println("\033[94m[Github Upload]\033[0m - deleting asset from", url)

	client := http.Client{}

	req, err := http.NewRequest("DELETE", url, nil)
	if err != nil {
		return err
	}

	req.SetBasicAuth(env.githubUser, env.githubToken)

	resp, err := client.Do(req)
	if err != nil {
		return err
	}
	defer resp.Body.Close()
	b, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return err
	}
	switch resp.StatusCode {
	case 200, 204:
		return nil
	}
	return fmt.Errorf("\033[94m[Github Upload]\033[0m - Asset delete failed: %s HTTP status code: %d", string(b), resp.StatusCode)
}

func (env Env) uploadGithubAsset(releaseID int64) (*GithubReleaseAsset, error) {
	assets, err := env.fetchGithubAssets(releaseID)
	if err != nil {
		return nil, err
	}

	for _, asset := range assets {
		if asset.Name == env.installerFilename {
			if err := env.deleteGithubAsset(asset.ID); err != nil {
				return nil, err
			}
		}
	}

	// Upload new asset
	url := fmt.Sprintf("https://uploads.github.com/repos/toggl-open-source/toggldesktop/releases/%d/assets?name=%s",
		releaseID, env.installerFilename)
	fmt.Println("\033[94m[Github Upload]\033[0m - Uploading asset to", url)

	b, err := ioutil.ReadFile(env.installer)
	if err != nil {
		return nil, err
	}

	buf := bytes.NewBuffer(b)

	client := http.Client{}

	req, err := http.NewRequest("POST", url, buf)
	if err != nil {
		return nil, err
	}

	req.Header.Set("Content-Type", "application/octet-stream")
	req.SetBasicAuth(env.githubUser, env.githubToken)

	resp, err := client.Do(req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return nil, err
	}
	fmt.Println(string(body))
	switch resp.StatusCode {
	case 422:
		return nil, fmt.Errorf("\033[94m[Github Upload]\033[0m - Warning! Asset %s already exists. It will NOT be updated",
			env.installerFilename)
	case 201:
		var asset GithubReleaseAsset
		if err := json.Unmarshal(body, &asset); err != nil {
			return nil, err
		}
		fmt.Println(asset)
		return &asset, nil
	}
	return nil, fmt.Errorf("\033[94m[Github Upload]\033[0m - Asset upload failed: %s HTTP status code: %d", string(body), resp.StatusCode)
}

var (
	platformFlag  = flag.String("platform", "", "platform name. Example: linux")
	versionFlag   = flag.String("version", "", "version, without prefix. Example: 0.1.2")
	installerFlag = flag.String("installer", "", "installer file name. Example: toggldesktop.zip")
	urlFlag       = flag.String("url", "", "Github asset link. Example: https://github.com/toggl-open-source/toggldesktop/releases/download/v7.3.69/toggldesktop_linux_x86_64-7_3_69.tar.gz")
)

func (reg *TogglBackendRegistration) parse() error {
	reg.channel = *channelFlag

	if !isSupportedChannel(reg.channel) {
		return errors.New("missing or invalid channel parameter")
	}
	reg.platform = *platformFlag
	if !isSupportedPlatform(reg.platform) {
		return errors.New("missing platform parameter")
	}
	reg.version = *versionFlag
	if len(reg.version) == 0 {
		return errors.New("missing version parameter")
	}
	reg.installer = *installerFlag
	if len(reg.installer) == 0 {
		return errors.New("missing installer parameter")
	}
	reg.url = *urlFlag
	if len(reg.url) == 0 {
		return errors.New("missing url parameter")
	}
	return nil
}

// Register installer in Toggl backend.
// This will be replaced with a Github-only
// workflow soon.
func doTogglBackendRegistration() error {
	var reg TogglBackendRegistration
	if err := reg.parse(); err != nil {
		return err
	}
	publicLocation, err := reg.registerInstaller()
	if err != nil {
		return err
	}
	fmt.Println("[Github Upload] - Public location (share this link!):", publicLocation)
	return nil
}

func doGithubRelease() error {
	var env Env
	if err := env.parse(); err != nil {
		return err
	}
	if err := env.createGithubRelease(); err != nil {
		return err
	}
	releaseID, err := env.getGithubReleaseID()
	if err != nil {
		return err
	}
	asset, err := env.uploadGithubAsset(releaseID)
	if err != nil {
		return err
	}
	fmt.Println("\033[94m[Github Upload]\033[0m - Asset:", asset)
	if asset != nil {
		fmt.Println("\033[94m[Github Upload]\033[0m - Asset URL:", asset.BrowserDownloadURL)
	} else {
		fmt.Println("\033[94m[Github Upload]\033[0m - Warning! No asset received from Github")
	}

	// Execute register only for supported platforms
	if !isSupportedPlatform(env.platform) {
		// Register installer in Toggl backend.
		// This will be replaced with a Github-only
		// workflow soon.
		reg := TogglBackendRegistration{
			url:       asset.BrowserDownloadURL,
			version:   env.prefixLessVersion,
			platform:  env.platform,
			installer: env.installer,
		}
		var publicLocation string
		publicLocation, err = reg.registerInstaller()
		if err != nil {
			return err
		}
		fmt.Println("[Github Upload] - Public location (share this link!):", publicLocation)
	}

	return nil
}

func calculateMd5Sum(filename string) (string, error) {
	f, err := os.Open(filename)
	if err != nil {
		return "", err
	}

	defer f.Close()
	hasher := md5.New()
	io.Copy(hasher, f)
	result := hex.EncodeToString(hasher.Sum(nil))
	return result, nil
}

// Registers the installer in Toggl backend
// and returns the *public* URL to Toggl backend that
// will hand over the installer.
func (reg TogglBackendRegistration) registerInstaller() (string, error) {
	md5sum, err := calculateMd5Sum(reg.installer)
	if err != nil {
		return "", nil
	}

	if len(reg.channel) == 0 {
		reg.channel = "dev"
	}

	url := fmt.Sprintf("https://www.toggl.com/api/v8/register_installer?app=td&channel=%s&platform=%s&version=%s&token=%s&url=%s&md5=%s",
		reg.channel, reg.platform, reg.version, installationRegistrationToken, reg.url, md5sum)
	fmt.Println("[Github Upload] - Registering installer with", url)
	resp, err := http.Get(url)
	if err != nil {
		return "", err
	}
	defer resp.Body.Close()
	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return "", err
	}
	if 200 == resp.StatusCode {
		fmt.Println("[Github Upload] - Installer registered:", string(body))
	} else {
		fmt.Println("[Github Upload] - Installer registration failed:", string(body), "HTTP status code", resp.StatusCode)
	}
	result := fmt.Sprintf("https://www.toggl.com/api/v8/installer?app=td&channel=%s&platform=%s", reg.channel, reg.platform)
	return result, nil
}