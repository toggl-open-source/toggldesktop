//
//  CalendarFlowLayout.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/24/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class CalendarFlowLayout: NSCollectionViewFlowLayout {

    // MARK: Variables

    private var cellAttributes: [NSCollectionViewLayoutAttributes] = []
    private let numberOfColumns = 7
    private var numberOfItems = 0

    // MARK: Init

    override init() {
        super.init()
        initCommon()
    }

    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        initCommon()
    }

    private func initCommon() {

        // Default size
        itemSize = CGSize(width: 31, height: 32)
        minimumLineSpacing = 20
        minimumInteritemSpacing = 2
        sectionInset = NSEdgeInsets(top: 4, left: 8, bottom: 5, right: 8)
        scrollDirection = .vertical
    }

    // MARK: Override

    override func prepare() {
        super.prepare()
        guard let collectionView = self.collectionView,
            let dataSource = collectionView.dataSource else { return }

        numberOfItems = dataSource.collectionView(collectionView, numberOfItemsInSection: 0)
        var atts: [NSCollectionViewLayoutAttributes] = []
        for i in 0..<numberOfItems {
            let indexPath = IndexPath(item: i, section: 0)
            let att = NSCollectionViewLayoutAttributes(forItemWith: indexPath)
            let col = i % numberOfColumns
            let row = i / numberOfColumns
            let x = sectionInset.left + CGFloat(col) * (itemSize.width + minimumInteritemSpacing)
            let y = sectionInset.right + CGFloat(row) * (itemSize.height + minimumLineSpacing)
            att.frame = CGRect(x: x, y: y, width: itemSize.width, height: itemSize.height)
            atts.append(att)
        }
        cellAttributes = atts
    }

    override func layoutAttributesForElements(in rect: NSRect) -> [NSCollectionViewLayoutAttributes] {
        return cellAttributes.compactMap({ (att) -> NSCollectionViewLayoutAttributes? in
            if att.frame.intersects(rect) {
                return att
            }
            return nil
        })
    }

    override func layoutAttributesForItem(at indexPath: IndexPath) -> NSCollectionViewLayoutAttributes? {
        return cellAttributes[indexPath.item]
    }

    override var collectionViewContentSize: NSSize {
        let row = numberOfItems / numberOfColumns
        let width = sectionInset.left + sectionInset.right + CGFloat(numberOfColumns) * (itemSize.width + minimumInteritemSpacing)
        let height = sectionInset.top + sectionInset.bottom + CGFloat(row) * (itemSize.height + minimumLineSpacing)
        return CGSize(width: width, height: height)
    }
}
