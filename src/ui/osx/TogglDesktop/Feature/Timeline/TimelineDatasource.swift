//
//  TimelineDatasource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class TimelineDatasource: NSObject {

    fileprivate struct Constants {
        static let TimeLabelCellID = NSUserInterfaceItemIdentifier("TimelineTimeLabelCell")
        static let TimeLabelCellXIB = NSNib.Name("TimelineTimeLabelCell")
        static let TimeEntryCellID = NSUserInterfaceItemIdentifier("TimelineTimeEntryCell")
        static let TimeEntryCellXIB = NSNib.Name("TimelineTimeEntryCell")
        static let ActivityCellID = NSUserInterfaceItemIdentifier("TimelineActivityCell")
        static let ActivityCellXIB = NSNib.Name("TimelineActivityCell")
    }

    enum ZoomLevel: Int {
        case x1 = 0 // normal
        case x2
        case x3
        case x4

        var span: TimeInterval {
            switch self {
            case .x4:
                return 7200.0 // Each 2 hours
            case .x1,
                .x2,
                .x3:
                return 3600 // Each 1 hour
            }
        }

        var nextLevel: ZoomLevel? {
            return ZoomLevel(rawValue: self.rawValue + 1)
        }

        var previousLevel: ZoomLevel? {
            return ZoomLevel(rawValue: self.rawValue - 1)
        }
    }

    // MARK: Variables

    private unowned let collectionView: NSCollectionView
    private let flow: TimelineFlowLayout
    private var timeline: TimelineData?
    private var zoomLevel: ZoomLevel = .x1
    
    // MARK: Init

    init(_ collectionView: NSCollectionView) {
        self.collectionView = collectionView
        self.flow = TimelineFlowLayout()
        super.init()
        flow.flowDelegate = self
        collectionView.delegate = self
        collectionView.dataSource = self
        collectionView.collectionViewLayout = flow
        collectionView.register(NSNib(nibNamed: Constants.TimeLabelCellXIB, bundle: nil), forItemWithIdentifier: Constants.TimeLabelCellID)
        collectionView.register(NSNib(nibNamed: Constants.TimeEntryCellXIB, bundle: nil), forItemWithIdentifier: Constants.TimeEntryCellID)
        collectionView.register(NSNib(nibNamed: Constants.ActivityCellXIB, bundle: nil), forItemWithIdentifier: Constants.ActivityCellID)
    }

    func render(_ timeline: TimelineData) {
        self.timeline = timeline
        collectionView.reloadData()
    }

    func update(_ zoomLevel: ZoomLevel) {
        self.zoomLevel = zoomLevel
        timeline?.render(with: zoomLevel)
        flow.apply(zoomLevel)
        collectionView.reloadData()
    }
}

extension TimelineDatasource: NSCollectionViewDataSource, NSCollectionViewDelegateFlowLayout {

    func numberOfSections(in collectionView: NSCollectionView) -> Int {
        guard let timeline = timeline else { return 0 }
        return timeline.numberOfSections
    }

    func collectionView(_ collectionView: NSCollectionView, numberOfItemsInSection section: Int) -> Int {
        guard let timeline = timeline else { return 0 }
        return timeline.numberOfItems(in: section)
    }

    func collectionView(_ collectionView: NSCollectionView, itemForRepresentedObjectAt indexPath: IndexPath) -> NSCollectionViewItem {
        guard let timeline = timeline,
            let section = TimelineData.Section(rawValue: indexPath.section),
            let item = timeline.item(at: indexPath) else { return NSCollectionViewItem() }

        switch section {
        case .timeLabel:
            let cell = collectionView.makeItem(withIdentifier: Constants.TimeLabelCellID, for: indexPath) as! TimelineTimeLabelCell
            let chunk = item as! TimelineTimeChunk
            cell.render(chunk)
            return cell
        case .timeEntry:
            let cell = collectionView.makeItem(withIdentifier: Constants.TimeEntryCellID, for: indexPath) as! TimelineTimeEntryCell
            return cell
        case .activity:
            let cell = collectionView.makeItem(withIdentifier: Constants.ActivityCellID, for: indexPath) as! TimelineActivityCell
            return cell
        }
    }
}

// MARK: TimelineFlowLayoutDelegate

extension TimelineDatasource: TimelineFlowLayoutDelegate {

    func timestampForItem(at indexPath: IndexPath) -> Timestamp? {
        return timeline?.timestampForItem(at: indexPath)
    }
}
