//
//  CalendarViewController.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/19/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class CalendarViewController: NSViewController {

    private struct Constants {

        static let cellID = NSUserInterfaceItemIdentifier("DateCell")
        static let cellNibName = NSNib.Name("DateCell")
    }

    // MARK: OUTLET

    @IBOutlet weak var collectionView: NSCollectionView!

    // MARK: Variables

    fileprivate lazy var dataSource: CalendarDataSource = CalendarDataSource(selectedDate)
    private var selectedDate = Date() {
        didSet {
            dataSource.selectedDate = selectedDate
        }
    }

    // MARK: View Cycle

    override func viewDidLoad() {
        super.viewDidLoad()
        initCommon()
        initCollectionView()
    }

    // MARK: Public

    func prepareLayout(with date: Date) {
        selectedDate = date
    }
}

// MARK: Private

extension CalendarViewController {

    fileprivate func initCommon() {

    }

    fileprivate func initCollectionView() {
        collectionView.register(NSNib(nibNamed: Constants.cellNibName, bundle: nil),
                                forItemWithIdentifier: Constants.cellID)
        collectionView.dataSource = dataSource
        collectionView.delegate = dataSource
        let flow = NSCollectionViewFlowLayout()
        flow.itemSize = CGSize(width: 32, height: 26)
        flow.minimumLineSpacing = 20
        flow.minimumInteritemSpacing = 2
        collectionView.collectionViewLayout = flow
    }
}
