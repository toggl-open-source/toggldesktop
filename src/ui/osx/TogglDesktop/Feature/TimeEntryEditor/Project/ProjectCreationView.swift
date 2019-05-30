//
//  ProjectCreationView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/28/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa
import Carbon.HIToolbox

protocol ProjectCreationViewDelegate: class {

    func projectCreationDidCancel()
    func projectCreationDidAdd(with name: String, color: String, projectGUID: String)
    func projectCreationDidUpdateSize()
}

final class ProjectCreationView: NSView {

    enum DisplayMode {
        case normal
        case compactColorPicker // Predefined colors
        case fullColorPicker // Predefined colors + wheel color

        var height: CGFloat {
            switch self {
            case .normal:
                return 200.0
            case .fullColorPicker:
                return 400.0
            case .compactColorPicker:
                return 300.0
            }
        }
    }

    // MARK: OUTLET

    @IBOutlet weak var addBtn: CursorButton!
    @IBOutlet weak var cancelBtn: CursorButton!
    @IBOutlet weak var projectTextField: NSTextField!
    @IBOutlet weak var workspaceAutoComplete: WorkspaceAutoCompleteTextField!
    @IBOutlet weak var clientAutoComplete: ClientAutoCompleteTextField!
    @IBOutlet weak var colorBtn: CursorButton!
    @IBOutlet weak var colorPickerContainerBox: NSBox!
    @IBOutlet weak var publicProjectCheckBox: NSButton!
    
    // MARK: Variables

    var selectedTimeEntry: TimeEntryViewItem! {
        didSet {
            resetViews()
        }
    }
    private(set) var selectedWorkspace: Workspace? {
        didSet {
            clientDatasource.selectedWorkspace = selectedWorkspace

            // Reset the current client
            if oldValue?.ID != selectedWorkspace?.ID {
                selectedClient = nil
                clientAutoComplete.stringValue = ""
            }

            // Update Premium plan
            isPremiumWorkspace = selectedWorkspace?.isPremium ?? false
        }
    }
    private var selectedClient: Client?
    private var isPublic = false
    private lazy var clientDatasource: ClientDataSource = {
        let firstWorkspace = workspaceDatasource.items.first as? Workspace
        let clients = ClientStorage.shared.getClients(at: firstWorkspace)
        return ClientDataSource(items: clients,
                                updateNotificationName: .ClientStorageChangedNotification)
    }()
    private lazy var workspaceDatasource = WorkspaceDataSource(items: WorkspaceStorage.shared.workspaces,
                                                               updateNotificationName: .WorkspaceStorageChangedNotification)
    weak var delegate: ProjectCreationViewDelegate?
    private var originalColor = ProjectColor.default
    private var selectedColor = ProjectColor.default {
        didSet {
            updateSelectColorView()
        }
    }
    private lazy var colorPickerView: ColorPickerView = {
        let picker = ColorPickerView.xibView() as ColorPickerView
        picker.delegate = self
        colorPickerContainerBox.addSubview(picker)
        picker.edgesToSuperView()
        return picker
    }()
    private var displayMode = DisplayMode.normal {
        didSet {
            updateLayout()
        }
    }
    private var isPremiumWorkspace = false {
        didSet {
            updateWorkspacePlanLayout()

            // Reset custom color if we switch Premium -> Free
            if oldValue && !isPremiumWorkspace {
                colorPickerView.resetBtnOnTap(self)
            }
        }
    }
    var suitableHeight: CGFloat {
        return displayMode.height
    }
    var isValidDataForProjectCreation: Bool {
        return selectedWorkspace != nil && !projectTextField.stringValue.isEmpty
    }

    // MARK: Public

    override func awakeFromNib() {
        super.awakeFromNib()

        initCommon()
        selecteFirstWorkspace()
        updateLayoutState()
    }

    func setTitleAndFocus(_ title: String) {
        projectTextField.stringValue = title
        window?.makeFirstResponder(projectTextField)
        updateLayoutState()
    }

    @IBAction func cancelBtnOnTap(_ sender: Any) {
        closeAllSuggestions()
        delegate?.projectCreationDidCancel()
    }

    private func getSelectedClientData() -> (UInt64, String?) {
        if let selectedClient = ClientStorage.shared.client(with: clientAutoComplete.stringValue) {
            return (selectedClient.ID, selectedClient.guid)
        }
        return (0, nil)
    }

    @IBAction func addBtnOnTap(_ sender: Any) {
        guard isValidDataForProjectCreation else { return }
        guard let selectedWorkspace = selectedWorkspace else {
            return
        }

        closeAllSuggestions()
        let clientData = getSelectedClientData()

        // Safe for unwrapped
        let isBillable = selectedTimeEntry.billable
        let timeEntryGUID = selectedTimeEntry.guid!
        let workspaceID = selectedWorkspace.ID
        let clientID = clientData.0
        let clientGUID = clientData.1
        let projectName = projectTextField.stringValue
        let colorHex = selectedColor.colorHex

        let projectGUID = DesktopLibraryBridge.shared().createProject(withTimeEntryGUID: timeEntryGUID,
                                                                    workspaceID: workspaceID,
                                                                    clientID: clientID,
                                                                    clientGUID: clientGUID,
                                                                    projectName: projectName,
                                                                    colorHex: colorHex,
                                                                    isPublic: isPublic)
        if selectedTimeEntry.billable {
            DesktopLibraryBridge.shared().setBillableForTimeEntryWithTimeEntryGUID(timeEntryGUID,
                                                                                   isBillable: isBillable)
        }

        delegate?.projectCreationDidAdd(with: projectName, color: colorHex, projectGUID: projectGUID)
    }

    @IBAction func publicProjectOnChange(_ sender: Any) {
        isPublic = publicProjectCheckBox.state == .on
    }

    @IBAction func colorBtnOnTap(_ sender: Any) {
        let isON = colorBtn.state == .on
        if isON {
            displayMode = isPremiumWorkspace ? .fullColorPicker : .compactColorPicker
        } else {
            displayMode = .normal
        }
        colorBtn.layer?.borderWidth = isON ? 4.0 : 0.0
    }

    override func keyDown(with event: NSEvent) {
        super.keyDown(with: event)

        // Close if need
        if event.keyCode == UInt16(kVK_Escape) {
            cancelBtnOnTap(self)
        }
    }
}

// MARK: Private

extension ProjectCreationView {

    fileprivate func initCommon() {
        colorPickerView.isHidden = false
        colorPickerContainerBox.isHidden = true
        colorBtn.wantsLayer = true
        colorBtn.layer?.cornerRadius = 12.0
        colorBtn.layer?.borderColor = colorBtnBorderColor.cgColor
        colorBtn.cursor = .pointingHand
        cancelBtn.cursor = .pointingHand
        addBtn.cursor = .pointingHand
        
        // Default value
        selectedColor = ProjectColor.default
        displayMode = .normal

        // Delegate
        projectTextField.delegate = self
        clientAutoComplete.autoCompleteDelegate = self

        // Setup data source
        clientDatasource.delegate = self
        clientDatasource.setup(with: clientAutoComplete)
        workspaceDatasource.delegate = self
        workspaceDatasource.setup(with: workspaceAutoComplete)

        // Arrow
        workspaceAutoComplete.layoutArrowBtn(with: self)
        clientAutoComplete.layoutArrowBtn(with: self)

        colorPickerContainerBox.wantsLayer = true
        colorPickerContainerBox.layer?.masksToBounds = true
        colorPickerContainerBox.layer?.cornerRadius = 8

        // Hide color wheel for free workspace by default
        isPremiumWorkspace = false
    }

    fileprivate func updateLayout() {
        let height = displayMode.height
        switch displayMode {
        case .normal:
            colorPickerContainerBox.isHidden = true
            self.frame = CGRect(x: frame.origin.x, y: frame.origin.y, width: frame.size.width, height: height)
        case .compactColorPicker,
             .fullColorPicker:
            colorPickerContainerBox.isHidden = false
            self.frame = CGRect(x: frame.origin.x, y: frame.origin.y, width: frame.size.width, height: height)
        }
        delegate?.projectCreationDidUpdateSize()
    }

    fileprivate func updateSelectColorView() {
        colorBtn.layer?.backgroundColor = ConvertHexColor.hexCode(toNSColor: selectedColor.colorHex)!.cgColor
        colorPickerView.select(selectedColor)
    }

    fileprivate var colorBtnBorderColor: NSColor {
        if #available(OSX 10.13, *) {
            return NSColor(named: "color-project-btn-border-color")!
        } else {
            return NSColor(white: 0, alpha: 0.1)
        }
    }

    fileprivate func createNewClient(with name: String) {
        guard !name.isEmpty else { return }

        // Focus to workspace text field if user hasn't selected any workspace
        guard let workspace = selectedWorkspace else {
            window?.makeFirstResponder(workspaceAutoComplete)
            return
        }

        let newClientGUID = DesktopLibraryBridge.shared().createClient(withWorkspaceID: workspace.ID, clientName: name)

        // Create fake client
        // because -createClient only return the client GUID and there is no way to construct the real ViewItem
        let newItem = ViewItem()
        newItem.guid = newClientGUID
        newItem.name = name
        self.selectedClient = Client(viewItem: newItem)

        // Update
        updateLayoutState()
    }

    fileprivate func updateLayoutState() {
        guard isValidDataForProjectCreation else {
            addBtn.isEnabled = false
            return
        }

        addBtn.isEnabled = true
    }

    fileprivate func selecteFirstWorkspace() {
        guard !workspaceDatasource.items.isEmpty else { return }
        workspaceDatasource.selectRow(at: 0)
    }

    fileprivate func resetViews() {
        selecteFirstWorkspace()
        clientAutoComplete.stringValue = ""
        selectedClient = nil
        selectedColor = ProjectColor.default
        updateLayoutState()
    }

    fileprivate func closeAllSuggestions() {
        workspaceAutoComplete.closeSuggestion()
        clientAutoComplete.closeSuggestion()
    }

    fileprivate func updateWorkspacePlanLayout() {
        colorPickerView.setColorWheelHidden(!isPremiumWorkspace)
        if displayMode != .normal {
            displayMode = isPremiumWorkspace ? .fullColorPicker : .compactColorPicker
        }
    }
}

// MARK: ColorPickerViewDelegate

extension ProjectCreationView: ColorPickerViewDelegate {

    func colorPickerDidSelectColor(_ color: ProjectColor) {
        selectedColor = color
    }

    func colorPickerShouldResetColor() {
        selectedColor = originalColor
    }
}

// MARK: AutoCompleteViewDataSourceDelegate

extension ProjectCreationView: AutoCompleteViewDataSourceDelegate {

    func autoCompleteSelectionDidChange(sender: AutoCompleteViewDataSource, item: Any) {
        if sender == clientDatasource {
            guard let client = item as? Client else { return }
            self.selectedClient = client
            clientAutoComplete.stringValue = client.name
            clientAutoComplete.closeSuggestion()
        }
        if sender == workspaceDatasource {
            guard let workspace = item as? Workspace else { return }
            self.selectedWorkspace = workspace
            workspaceAutoComplete.stringValue = workspace.name
            workspaceAutoComplete.closeSuggestion()
        }

        // Update add button
        updateLayoutState()
    }
}

// MARK: AutoCompleteTextFieldDelegate

extension ProjectCreationView: AutoCompleteTextFieldDelegate {

    func autoCompleteDidTapOnCreateButton(_ sender: AutoCompleteTextField) {
        if sender == clientAutoComplete {
            clientAutoComplete.closeSuggestion()
            createNewClient(with: clientAutoComplete.stringValue)
            updateLayoutState()
        }
    }

    func autoCompleteViewDidClose(_ sender: AutoCompleteTextField) {}

    func shouldClearCurrentSelection(_ sender: AutoCompleteTextField) {}

    func autoCompleteTextFieldDidEndEditing(_ sender: AutoCompleteTextField) {}

    func autoCompleteShouldCloseEditor(_ sender: AutoCompleteTextField) {
        cancelBtnOnTap(self)
    }
}

// MARK: NSTextFieldDelegate

extension ProjectCreationView: NSTextFieldDelegate {

    func controlTextDidChange(_ obj: Notification) {
        if let textField = obj.object as? NSTextField, textField == projectTextField {
            updateLayoutState()
        }
    }

    func control(_ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector) -> Bool {

        // Escape
        if commandSelector == #selector(NSResponder.cancelOperation(_:)) {
            cancelBtnOnTap(self)
            return true
        }

        return false
    }
}
