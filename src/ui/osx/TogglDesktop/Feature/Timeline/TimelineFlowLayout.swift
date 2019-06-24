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

    // MARK: Variables

    weak var flowDelegate: TimelineFlowLayoutDelegate?
    private var zoomLevel: TimelineDatasource.ZoomLevel = .x1
    private var timeLabelSize = CGSize(width: 54.0, height: 32)
    private var timeLablesAttributes: [NSCollectionViewLayoutAttributes] = []
    private var timeEntryAttributes: [NSCollectionViewLayoutAttributes] = []
    private var activityAttributes: [NSCollectionViewLayoutAttributes] = []
    private var paddingTimeLabel: CGFloat {
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

        // Default size
        itemSize = CGSize(width: 31, height: 32)
        minimumLineSpacing = 0
        minimumInteritemSpacing = 0
        sectionInset = NSEdgeInsetsZero
        scrollDirection = .vertical
    }

    override func prepare() {
        super.prepare()
        guard let collectionView = self.collectionView,
            let dataSource = collectionView.dataSource,
            let flowDelegate = flowDelegate else { return }

        timeLablesAttributes = []
        timeEntryAttributes = []

        // Number of items
        numberOfTimeLabels = dataSource.collectionView(collectionView, numberOfItemsInSection: TimelineData.Section.timeLabel.rawValue)
        let numberOfTimeEntry = dataSource.collectionView(collectionView, numberOfItemsInSection: TimelineData.Section.timeEntry.rawValue)
        let numberOfActivity = dataSource.collectionView(collectionView, numberOfItemsInSection: TimelineData.Section.activity.rawValue)

        // Size

        // Time labels
        for i in 0..<numberOfTimeLabels {
            let indexPath = IndexPath(item: i, section: TimelineData.Section.timeLabel.rawValue)
            let att = NSCollectionViewLayoutAttributes(forItemWith: indexPath)
            let y: CGFloat = CGFloat(i) * (paddingTimeLabel + timeLabelSize.height)
            att.frame = CGRect(x: 0,
                               y: y,
                               width: timeLabelSize.width,
                               height: timeLabelSize.height)
            timeLablesAttributes.append(att)
        }

        // Time entries
        for i in 0..<numberOfTimeEntry {
            let indexPath = IndexPath(item: i, section: TimelineData.Section.timeLabel.rawValue)
            guard let timestamp = flowDelegate.timestampForItem(at: indexPath)
            let att = NSCollectionViewLayoutAttributes(forItemWith: indexPath)
        }
    }

    override func layoutAttributesForElements(in rect: NSRect) -> [NSCollectionViewLayoutAttributes] {
        return timeLablesAttributes.compactMap({ (att) -> NSCollectionViewLayoutAttributes? in
            if att.frame.intersects(rect) {
                return att
            }
            return nil
        })
    }

    override func layoutAttributesForItem(at indexPath: IndexPath) -> NSCollectionViewLayoutAttributes? {
        return timeLablesAttributes[indexPath.item]
    }

    override var collectionViewContentSize: NSSize {
        let width = collectionView?.bounds.width ?? 300.0
        let height = CGFloat(numberOfTimeLabels) * (paddingTimeLabel + timeLabelSize.height)
        return CGSize(width: width, height: height)
    }
}
