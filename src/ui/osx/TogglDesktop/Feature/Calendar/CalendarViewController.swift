//
//  CalendarViewController.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/19/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class CalendarViewController: NSViewController {

    // MARK: OUTLET

    @IBOutlet weak var collectionView: NSCollectionView!
    
    // MARK: Variables

    fileprivate lazy var dataSource: CalendarDataSource = CalendarDataSource(selectedDate: selectedDate)
    private var isViewAppearing = false
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

    override func viewDidAppear() {
        super.viewDidAppear()
        isViewAppearing = true
        reloadCalendarView()
    }

    override func viewWillDisappear() {
        super.viewWillDisappear()
        isViewAppearing = false
    }

    // MARK: Public

    func prepareLayout(with date: Date) {
        selectedDate = date
        reloadCalendarView()
    }

    private func reloadCalendarView() {
        if isViewAppearing {
            collectionView.reloadData()
            self.collectionView.scrollToItems(at: Set<IndexPath>.init(arrayLiteral: IndexPath(item: self.dataSource.indexForCurrentDate, section: 0)),
                                              scrollPosition: [.centeredVertically])
        }
    }
}

// MARK: Private

extension CalendarViewController {

    fileprivate func initCommon() {

    }

    fileprivate func initCollectionView() {
        collectionView.register(NSNib(nibNamed: CalendarDataSource.Constants.cellNibName, bundle: nil),
                                forItemWithIdentifier: CalendarDataSource.Constants.cellID)
        collectionView.dataSource = dataSource
        collectionView.delegate = dataSource
        let flow = NSCollectionViewFlowLayout()
        flow.itemSize = CGSize(width: 32, height: 26)
        flow.minimumLineSpacing = 20
        flow.minimumInteritemSpacing = 2
        collectionView.collectionViewLayout = flow
    }
}
