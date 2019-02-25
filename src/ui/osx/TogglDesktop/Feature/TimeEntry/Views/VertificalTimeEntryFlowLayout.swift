//
//  VertificalTimeEntryFlowLayout.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol VertificalTimeEntryFlowLayoutDelegate: class {

    func isLoadMoreItem(at section: Int) -> Bool
}

final class VertificalTimeEntryFlowLayout: NSCollectionViewFlowLayout {

    // MARK: Variables
    weak var delegate: VertificalTimeEntryFlowLayoutDelegate?
    private var decoratorAttributes: [NSCollectionViewLayoutAttributes] = []

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
        itemSize = NSSize(width: 280, height: 64)
        sectionInset = NSEdgeInsets(top: 0, left: 10, bottom: 10, right: 10)
        minimumInteritemSpacing = 0
        minimumLineSpacing = 0
        scrollDirection = .vertical
        headerReferenceSize = CGSize(width: 280, height: 36)
    }

    // MARK: Override

    override func prepare() {
        super.prepare()
        guard let collectionView = self.collectionView,
            let dataSource = collectionView.dataSource,
            let delegate = collectionView.delegate as? NSCollectionViewDelegateFlowLayout,
            let flowDelegate = self.delegate else { return }

        decoratorAttributes = []

        let numberOfSection = dataSource.numberOfSections?(in: collectionView) ?? 0

        if numberOfSection > 0 {
            let headerSize = delegate.collectionView?(collectionView, layout: self, referenceSizeForHeaderInSection: 0) ?? headerReferenceSize
            let cellSize = delegate.collectionView?(collectionView, layout: self, sizeForItemAt: IndexPath(item: 0, section: 0)) ?? itemSize

            // Calculate the size of decorator vuew
            var lastY: CGFloat = 0
            for i in 0..<numberOfSection {

                // We don't add Decorator view for loadMore
                guard !flowDelegate.isLoadMoreItem(at: i) else { continue }

                // Get the size
                let numberOfItems = dataSource.collectionView(collectionView, numberOfItemsInSection: i)
                let height = CGFloat(numberOfItems) * cellSize.height + headerSize.height

                let indexpath = IndexPath(item: 0, section: i)
                let layout = NSCollectionViewLayoutAttributes(forDecorationViewOfKind: "TimeDecoratorView", with: indexpath)
                layout.zIndex = -1
                layout.frame = NSRect(x: 0, y: lastY, width: collectionView.frame.width, height: height)
                decoratorAttributes.append(layout)

                lastY += height + sectionInset.bottom + sectionInset.top
            }
        }
    }

    override func layoutAttributesForDecorationView(ofKind elementKind: NSCollectionView.DecorationElementKind,
                                                    at indexPath: IndexPath) -> NSCollectionViewLayoutAttributes? {
        if elementKind == "TimeDecoratorView" {
            return decoratorAttributes[indexPath.section]
        }
        return nil
    }

    override func layoutAttributesForElements(in rect: NSRect) -> [NSCollectionViewLayoutAttributes] {
        var attributes = super.layoutAttributesForElements(in: rect)

        for att in decoratorAttributes {
            if rect.intersects(att.frame) {
                attributes.append(att)
            }
        }

        return attributes
    }
}
