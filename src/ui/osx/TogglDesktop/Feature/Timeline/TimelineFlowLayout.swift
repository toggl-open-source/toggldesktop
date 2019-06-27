//
//  TimelineFlowLayout.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol TimelineFlowLayoutDelegate: class {

    func timestampForItem(at indexPath: IndexPath) -> Timestamp?
}

final class TimelineFlowLayout: NSCollectionViewFlowLayout {

    private struct Constants {
        static let MinimumHeightOfTimeEntry: CGFloat = 2.0

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
        }

        struct Activity {
            static let RightPadding: CGFloat = 25.0
            static let Width: CGFloat = 20.0
        }
    }

    // MARK: Variables

    weak var flowDelegate: TimelineFlowLayoutDelegate?
    private var zoomLevel: TimelineDatasource.ZoomLevel = .x1
    private var timeLablesAttributes: [NSCollectionViewLayoutAttributes] = []
    private var timeEntryAttributes: [NSCollectionViewLayoutAttributes] = []
    private var activityAttributes: [NSCollectionViewLayoutAttributes] = []
    private var dividerAttributes: [NSCollectionViewLayoutAttributes] = []
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

    // MARK: Override

    override init() {
        super.init()
        initCommon()
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
        numberOfTimeLabels = dataSource.collectionView(collectionView, numberOfItemsInSection: TimelineData.Section.timeLabel.rawValue)
        numberOfTimeEntry = dataSource.collectionView(collectionView, numberOfItemsInSection: TimelineData.Section.timeEntry.rawValue)
        numberOfActivity = dataSource.collectionView(collectionView, numberOfItemsInSection: TimelineData.Section.activity.rawValue)

        // MARK: Calculation

        calculateDividerAttributes()
        calculateTimeLabelAttributes()
        calculateTimeEntryAttributes()
        calculateActivityAttributes()
    }

    override func layoutAttributesForElements(in rect: NSRect) -> [NSCollectionViewLayoutAttributes] {
        let allAttributes = timeLablesAttributes + timeEntryAttributes + activityAttributes + dividerAttributes + activityAttributes
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
        }
    }

    override func layoutAttributesForSupplementaryView(ofKind elementKind: NSCollectionView.SupplementaryElementKind, at indexPath: IndexPath) -> NSCollectionViewLayoutAttributes? {
        if elementKind == NSCollectionView.elementKindSectionFooter {
            return dividerAttributes[safe: indexPath.section]
        }
        return nil
    }

    override var collectionViewContentSize: NSSize {
        let width = collectionView?.bounds.width ?? 300.0
        let height = CGFloat(numberOfTimeLabels) * (verticalPaddingTimeLabel + Constants.TimeLabel.Size.height)
        return CGSize(width: width, height: height)
    }

    private func getTimestampForStartOfDay(from timestamp: TimeInterval) -> TimeInterval {
        let date = Date(timeIntervalSince1970: timestamp)
        var calendar = Calendar.current
        calendar.timeZone = TimeZone(abbreviation: "UTC")!
        return calendar.startOfDay(for: date).timeIntervalSince1970
    }
}

// MARK: Private

extension TimelineFlowLayout {

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
            timeLablesAttributes.append(att)
        }
    }

    func calculateTimeEntryAttributes() {
        for i in 0..<numberOfTimeEntry {
            let indexPath = IndexPath(item: i, section: TimelineData.Section.timeEntry.rawValue)
            guard let timestamp = flowDelegate?.timestampForItem(at: indexPath) else {
                print("Missing timestamp for at \(indexPath)")
                continue
            }

            let att = NSCollectionViewLayoutAttributes(forItemWith: indexPath)
            let beginDay = getTimestampForStartOfDay(from: timestamp.start)

            // Length of time entry
            let span = CGFloat(timestamp.end - timestamp.start)

            // The ratio
            // From the design, the size of time label + padding prepresents 1 hours or 2 hours (depend on zoomLevel)
            // Get the ratio for later calculations
            let ratio = (Constants.TimeLabel.Size.height + verticalPaddingTimeLabel) / CGFloat(zoomLevel.span)

            // Ex: To calculate the height of the entry with X timestamp
            // Height = X * ratio
            let y = CGFloat((timestamp.start - beginDay)) * ratio

            // The minimum height is 2.0 pixel
            let height = CGFloat.maximum(span * ratio, Constants.MinimumHeightOfTimeEntry)

            var frame = CGRect.zero

            // Check if this time entry intersect with previous one
            // If overlap, increase the number of columns
            var col = 0
            var intersected = false
            repeat {
                let x = Constants.TimeEntry.LeftPadding + CGFloat(col) * (Constants.TimeEntry.Width + Constants.TimeEntry.RightPadding)
                frame = CGRect(x: x,
                               y: y,
                               width: Constants.TimeEntry.Width,
                               height: height)

                // Travesal all previous TimeEntry, if it's overlapped -> return
                // O(n) = n
                // It's reasonal since the number of items is small
                intersected = timeEntryAttributes.contains { (attribute) -> Bool in
                    return attribute.frame.intersects(frame)
                }

                // If overlap -> Move to next column
                if intersected {
                    col += 1
                }
            } while intersected

            // Finalize
            att.frame = frame
            timeEntryAttributes.append(att)
        }
    }

    fileprivate func calculateActivityAttributes() {
        let size = collectionViewContentSize
        for i in 0..<numberOfActivity {
            let indexPath = IndexPath(item: i, section: TimelineData.Section.activity.rawValue)
            let att = NSCollectionViewLayoutAttributes(forItemWith: indexPath)
            let size = Constants.TimeLabel.Size
            let x = size.width - Constants.Activity.RightPadding
            let y: CGFloat = CGFloat(i) * (verticalPaddingTimeLabel + size.height)
            att.frame = CGRect(x: x,
                               y: y,
                               width: size.width,
                               height: size.height)
            timeLablesAttributes.append(att)
        }
    }
}
