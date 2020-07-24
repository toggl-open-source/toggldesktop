import Danger

let danger = Danger()

SwiftLint.lint(.modifiedAndCreatedFiles(directory: nil), configFile: "src/ui/osx/TogglDesktop/.swiftlint.yml")
