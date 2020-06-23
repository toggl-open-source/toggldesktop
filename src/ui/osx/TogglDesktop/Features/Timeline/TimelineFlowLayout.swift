//
//  TimelineFlowLayout.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol TimelineFlowLayoutDelegate: class {

    func isEmptyTimeEntry(at indexPath: IndexPath) -> Bool
    func timechunkForItem(at indexPath: IndexPath) -> TimeChunk?
    func columnForItem(at indexPath: IndexPath) -> Int
    func shouldDrawDetailBubble(at indexPath: IndexPath) -> Bool
    func flowLayoutDidUpdateLayout(with activityFrame: CGRect)
}

/// Responsible for drawing all Timeline elements
/// Includes: Time Lable, Time Entry (Overlap), Gap Time Entry, Current Moment Line, Running Time Entry and Activity
/// Support Zoom
final class TimelineFlowLayout: NSCollectionViewFlowLayout {

    struct Constants {
        static let MinimumHeight: CGFloat = 2.0
        static let TimelineLineView = "TimelineLineView"
        static let TimelineLineViewX: CGFloat = 44
        static let TimelineLineViewHeight: CGFloat = 8
        static let TimelineLineInterval: TimeInterval = 60 // 1 min

        struct TimeLabel {
            static let Size = CGSize(width: 54.0, height: 32)
        }

        struct Divider {
            static let FirstDividerLeftPadding: CGFloat = 54.0
            static let SeconDividerRightPadding: CGFloat = 70.0
        }

        struct TimeEntry {
            static let LeftPadding: CGFloat = 65.0
            static let RightPadding: CGFloat = 10.0
            static let Width: CGFloat = 20.0
            static let TrailingPadding: CGFloat = 8.0
        }

        struct Activity {
            static let RightPadding: CGFloat = 25.0
            static let Width: CGFloat = 20.0
        }
    }

    // MARK: Variables

    weak var flowDelegate: TimelineFlowLayoutDelegate?
    var currentDate = Date()
    private var zoomLevel: TimelineDatasource.ZoomLevel = .x1
    private var timeLablesAttributes: [NSCollectionViewLayoutAttributes] = []
    private var timeEntryAttributes: [NSCollectionViewLayoutAttributes] = []
    private var activityAttributes: [NSCollectionViewLayoutAttributes] = []
    private var dividerAttributes: [NSCollectionViewLayoutAttributes] = []
    private var backgroundAttributes: [NSCollectionViewLayoutAttributes] = []
    private(set) var currentMomentAttribute: NSCollectionViewLayoutAttributes?
    private var verticalPaddingTimeLabel: CGFloat {
        switch zoomLevel {
        case .x1:
            return 150.0
        case .x2:
            return 64.0
        case .x3,
             .x4:
            return 20.0
        }
    }
    private var numberOfTimeLabels = 0
    private var numberOfTimeEntry = 0
    private var numberOfActivity = 0
    private var numberOfBackground = 0
    private var timer: Timer?

    // MARK: Override

    override init() {
        super.init()
        initCommon()
    }

    deinit {
        timer?.invalidate()
        timer = nil
    }

    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        initCommon()
    }

    func apply(_ zoomLevel: TimelineDatasource.ZoomLevel) {
        self.zoomLevel = zoomLevel
    }

    private func initCommon() {
        itemSize = CGSize(width: 32, height: 32)
        minimumLineSpacing = 0
        minimumInteritemSpacing = 0
        sectionInset = NSEdgeInsetsZero
        scrollDirection = .vertical
        register(NSNib(nibNamed: Constants.TimelineLineView, bundle: nil), forDecorationViewOfKind: Constants.TimelineLineView)
        timer = Timer.scheduledTimer(timeInterval: Constants.TimelineLineInterval, target: self, selector: #selector(self.updateCurrentMomemtLineOnTick), userInfo: nil, repeats: true)
    }

    override func prepare() {
        super.prepare()
        guard let collectionView = self.collectionView,
            let dataSource = collectionView.dataSource else { return }

        // Prepare
        timeLablesAttributes = []
        timeEntryAttributes = []
        dividerAttributes = []
        activityAttributes = []
        backgroundAttributes = []
        numberOfTimeLabels = dataSource.collectionView(collectionView, numberOfItemsInSection: TimelineData.Section.timeLabel.rawValue)
        numberOfTimeEntry = dataSource.collectionView(collectionView, numberOfItemsInSection: TimelineData.Section.timeEntry.rawValue)
        numberOfActivity = dataSource.collectionView(collectionView, numberOfItemsInSection: TimelineData.Section.activity.rawValue)
        numberOfBackground = dataSource.collectionView(collectionView, numberOfItemsInSection: TimelineData.Section.background.rawValue)

        // MARK: Calculation

        calculateDividerAttributes()
        calculateTimeLabelAttributes()
        calculateTimeEntryAttributes()
        calculateActivityAttributes()
        calculateBackgroundAttributes()
        calculateCurrentMomentAttribute()

        // Update the position of labels
        if let activitySection = dividerAttributes.last {
            flowDelegate?.flowLayoutDidUpdateLayout(with: activitySection.frame)
        }
    }

    override func layoutAttributesForElements(in rect: NSRect) -> [NSCollectionViewLayoutAttributes] {
        var allAttributes = timeLablesAttributes + timeEntryAttributes + activityAttributes + dividerAttributes + backgroundAttributes
        if let currentMomentAttribute = currentMomentAttribute {
            allAttributes.append(currentMomentAttribute)
        }
        return allAttributes.compactMap({ (att) -> NSCollectionViewLayoutAttributes? in
            if att.frame.intersects(rect) {
                return att
            }
            return nil
        })
    }

    override func layoutAttributesForItem(at indexPath: IndexPath) -> NSCollectionViewLayoutAttributes? {
        guard let section = TimelineData.Section(rawValue: indexPath.section) else { return nil }
        switch section {
        case .timeLabel:
            return timeLablesAttributes[safe: indexPath.item]
        case .timeEntry:
            return timeEntryAttributes[safe: indexPath.item]
        case .activity:
            return activityAttributes[safe: indexPath.item]
        case .background:
            return backgroundAttributes[safe: indexPath.item]
        }
    }

    override func layoutAttributesForSupplementaryView(ofKind elementKind: NSCollectionView.SupplementaryElementKind, at indexPath: IndexPath) -> NSCollectionViewLayoutAttributes? {
        if elementKind == NSCollectionView.elementKindSectionFooter {
            return dividerAttributes[safe: indexPath.section]
        }
        return nil
    }

    override func layoutAttributesForDecorationView(ofKind elementKind: NSCollectionView.DecorationElementKind, at indexPath: IndexPath) -> NSCollectionViewLayoutAttributes? {
        if elementKind == Constants.TimelineLineView {
            return currentMomentAttribute
        }
        return nil
    }

    override var collectionViewContentSize: NSSize {
        let width = collectionView?.bounds.width ?? 300.0
        let height = CGFloat(numberOfTimeLabels) * (verticalPaddingTimeLabel + Constants.TimeLabel.Size.height)
        return CGSize(width: width, height: height)
    }

    func isInTimeEntrySection(at location: CGPoint) -> Bool {
        guard let timeLabelDivider = dividerAttributes.first,
            let activityDivider = dividerAttributes.last else { return false }
        return timeLabelDivider.frame.origin.x < location.x && location.x < activityDivider.frame.origin.x
    }

    @objc private func updateCurrentMomemtLineOnTick() {
        guard currentMomentAttribute != nil else { return }

        // Invalid only decoration view (Current Line) in 1 min interval
        let context = NSCollectionViewFlowLayoutInvalidationContext()
        context.invalidateDecorationElements(ofKind: Constants.TimelineLineView, at: Set<IndexPath>(arrayLiteral: IndexPath(item: 0, section: 0)))
        self.invalidateLayout(with: context)
    }

    override func invalidateLayout(with context: NSCollectionViewLayoutInvalidationContext) {
        // Decoration for Current Line
        if let invalidatedDecorationIndexPaths = context.invalidatedDecorationIndexPaths,
            !invalidatedDecorationIndexPaths.isEmpty {
            calculateCurrentMomentAttribute()
        }
        super.invalidateLayout(with: context)
    }
}

// MARK: Private

extension TimelineFlowLayout {

    private var ratio: CGFloat {
        // The ratio
        // From the design, the size of time label + padding prepresents 1 hours or 2 hours (depend on zoomLevel)
        // Get the ratio for later calculations
        return (Constants.TimeLabel.Size.height + verticalPaddingTimeLabel) / CGFloat(zoomLevel.span)
    }

    /// Returns timestamp from a click point, depending on a zoom level and position
    public func convertTimestamp(from location: CGPoint) -> TimeInterval {
        let beginDay = Date.startOfDay(from: currentDate.timeIntervalSince1970)
        let start = TimeInterval((location.y / ratio)) + beginDay
        return start
    }

    func convertToYPosition(from timestamp: TimeInterval) -> CGFloat {
        let beginDay = Date.startOfDay(from: currentDate.timeIntervalSince1970)
        let y = CGFloat(timestamp - beginDay) * ratio
        return y
    }

    private func calculateBlockSize(at indexPath: IndexPath) -> (y: CGFloat, height: CGFloat)? {
        guard let timestamp = flowDelegate?.timechunkForItem(at: indexPath) else {
            print("Missing timestamp for at \(indexPath)")
            return nil
        }
        let beginDay = Date.startOfDay(from: timestamp.start)

        // Length of time entry
        let span = CGFloat(timestamp.end - timestamp.start)

        // Ex: To calculate the height of the entry with X timestamp
        // Height = X * ratio
        let y = CGFloat((timestamp.start - beginDay)) * ratio

        // The minimum height is 2.0 pixel
        let height = CGFloat.maximum(span * ratio, Constants.MinimumHeight)

        return (y, height)
    }

    private func getXDivider(at section: TimelineData.Section) -> CGFloat {
        switch section {
        case .timeLabel:
            return Constants.Divider.FirstDividerLeftPadding
        case .timeEntry:
            return collectionViewContentSize.width - Constants.Divider.SeconDividerRightPadding
        default:
            return 0
        }
    }

    fileprivate func calculateDividerAttributes() {
        let dividerSections: [TimelineData.Section] = [.timeLabel, .timeEntry]
        let dividerSectionsIndex = dividerSections.map { $0.rawValue }
        for i in dividerSectionsIndex {
            let indexPath = IndexPath(item: 0, section: i)
            let x = getXDivider(at: dividerSections[i])
            let frame = CGRect(x: x, y: 0, width: 1, height: collectionViewContentSize.height)
            let view = NSCollectionViewLayoutAttributes(forSupplementaryViewOfKind: NSCollectionView.elementKindSectionFooter, with: indexPath)
            view.frame = frame
            view.zIndex = 1
            dividerAttributes.append(view)
        }
    }

    fileprivate func calculateTimeLabelAttributes() {
        for i in 0..<numberOfTimeLabels {
            let indexPath = IndexPath(item: i, section: TimelineData.Section.timeLabel.rawValue)
            let att = NSCollectionViewLayoutAttributes(forItemWith: indexPath)
            let size = Constants.TimeLabel.Size
            let y: CGFloat = CGFloat(i) * (verticalPaddingTimeLabel + size.height)
            att.frame = CGRect(x: 0,
                               y: y,
                               width: size.width,
                               height: size.height)
            att.zIndex = 2
            timeLablesAttributes.append(att)
        }
    }

    private func calculateBackgroundAttributes() {
        let width = collectionViewContentSize.width
        for i in 0..<numberOfBackground {
            let labelIndex = i * 2
            let currentTimeLabel = timeLablesAttributes[labelIndex]

            let indexPath = IndexPath(item: i, section: TimelineData.Section.background.rawValue)
            let y = currentTimeLabel.frame.origin.y
            var height: CGFloat = 0

            // calculate height from current y to next label
            if let nextTimeLabel = timeLablesAttributes[safe: labelIndex + 1] {
                height = nextTimeLabel.frame.origin.y - y
            } else {
                height = collectionViewContentSize.height - y
            }

            // Init
            let frame = CGRect(x: 0,
                               y: y,
                               width: width,
                               height: height)
            let view = NSCollectionViewLayoutAttributes(forItemWith: indexPath)
            view.frame = frame
            view.zIndex = 0
            backgroundAttributes.append(view)
        }
    }

    private func calculateCurrentMomentAttribute() {
        let y = convertToYPosition(from: Date().timeIntervalSince1970) - Constants.TimelineLineViewHeight / 2.0
        let frame = CGRect(x: Constants.TimelineLineViewX, y: y, width: collectionView?.frame.width ?? 0, height: Constants.TimelineLineViewHeight)

        // If the current line is out of bounds
        // Just skip
        if frame.origin.y > collectionViewContentSize.height {
            self.currentMomentAttribute = nil
            return
        }

        // Create a line
        let att = NSCollectionViewLayoutAttributes(forDecorationViewOfKind: Constants.TimelineLineView, with: IndexPath(item: 0, section: 0))
        att.frame = frame
        att.zIndex = 1
        self.currentMomentAttribute = att
    }

    private func calculateTimeEntryAttributes() {
        for i in 0..<numberOfTimeEntry {
            let indexPath = IndexPath(item: i, section: TimelineData.Section.timeEntry.rawValue)

            // Calculate size, depend on timestamp
            guard let blockSize = calculateBlockSize(at: indexPath) else {
                continue
            }

            // Data
            let att = NSCollectionViewLayoutAttributes(forItemWith: indexPath)
            let y = blockSize.y
            let height = blockSize.height
            var frame = CGRect.zero

            // Check if this time entry intersect with previous one
            // If overlap, increase the number of columns
            let col = flowDelegate?.columnForItem(at: indexPath) ?? 0
            let x = Constants.TimeEntry.LeftPadding + CGFloat(col) * (Constants.TimeEntry.Width + Constants.TimeEntry.RightPadding)
            var width: CGFloat = 0
            if let shouldDrawBubble = flowDelegate?.shouldDrawDetailBubble(at: indexPath), shouldDrawBubble == true {
                width = getXDivider(at: TimelineData.Section.timeEntry) - x - Constants.TimeEntry.TrailingPadding
            } else {
                width = Constants.TimeEntry.Width
            }

            frame = CGRect(x: x,
                           y: y,
                           width: width,
                           height: height)

            // Finalize
            att.frame = frame
            att.zIndex = 3
            timeEntryAttributes.append(att)
        }

        // Hide all Empty TimeEntry if it's too small
        timeEntryAttributes.forEach { (att) in
            if let indexPath = att.indexPath,
                let isEmptyTimeEntry = flowDelegate?.isEmptyTimeEntry(at: indexPath),
                isEmptyTimeEntry == true {
                if att.frame.size.height <= 20 {
                    att.isHidden = true
                }
            }
        }
    }

    private func calculateActivityAttributes() {
        let contentSize = collectionViewContentSize
        for i in 0..<numberOfActivity {
            let indexPath = IndexPath(item: i, section: TimelineData.Section.activity.rawValue)
            let x = contentSize.width - Constants.Activity.RightPadding - Constants.Activity.Width
            guard let blockSize = calculateBlockSize(at: indexPath) else {
                print("Missing timestamp for at \(indexPath)")
                continue
            }

            let att = NSCollectionViewLayoutAttributes(forItemWith: indexPath)
            let y = blockSize.y
            let height = blockSize.height
            att.frame = CGRect(x: x,
                               y: y,
                               width: Constants.Activity.Width,
                               height: height)
            att.zIndex = 4
            activityAttributes.append(att)
        }
    }
}
