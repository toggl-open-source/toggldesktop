import Danger 
let danger = Danger()

let editedFiles = danger.git.modifiedFiles + danger.git.createdFiles
message("These files have changed: \(editedFiles.joined(separator: ", "))")

//SwiftLint.lint()

// Instead of making a markdown table in the main message
// sprinkle those comments inline, this can be a bit noisy
// but it definitely feels magical.
SwiftLint.lint(inline: true)

// Have different runs of SwiftLint against different sub-folders
//SwiftLint.lint(directory: "TogglDesktop", configFile: "TogglDesktop/.swiftlint.yml")

// The equivalent to running `swiftlint` in the root of the folder
//SwiftLint.lint(lintAllFiles: true)

// Use a different path for SwiftLint
//SwiftLint.lint(swiftlintPath: "Pods/SwiftLint/swiftlint")
