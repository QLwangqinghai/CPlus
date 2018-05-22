//
//  AppDelegate.swift
//  CPlusDemo
//
//  Created by wangqinghai on 2018/5/10.
//  Copyright © 2018年 wangqinghai. All rights reserved.
//

import Cocoa
import CPlusRuntime

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {

    @IBOutlet weak var window: NSWindow!


    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Insert code here to initialize your application
        
//        CPlus.
        let ptr = CPMemoryLoggerAllocInit(nil, nil) { (logger, item) in
            print("log action: \(item.pointee.action), code: \(item.pointee.code), type: \(item.pointee.type.pointee.base.name)")
        }
        var key: UInt32 = 0
        let result = CPAlloctorDefaultAddLogger(ptr, &key)
        print("result: \(result)")
        
        
        
    }

    func tt() {
        
        let ptr = CPMemoryLoggerAllocInit(nil, nil) { (logger, item) in
            print("log action: \(item.pointee.action), code: \(item.pointee.code), type: \(item.pointee.type.pointee.base.name)")
        }
        var key: UInt32 = 0
        let result = CPAlloctorDefaultAddLogger(ptr, &key)
        print("result: \(result)")
        CPAlloctorDefault
        
    }
    
    
    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
    }


}

