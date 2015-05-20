//
// begin license header
//
// File added by Manuj Naman Apr 2015
//
// end license header
//

#ifndef __USBSYMLINK_H__
#define __USBSYMLINK_H__

/** std includes */
#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <libusb.h>

#include <cstdlib>
#include <iostream>
#include <string>

  class UsbSymlink {
   public:
   
    // Symlink is used to identify a camera from another
    std::string symlink_;
    
    // Pixy camera Vendor and Product Ids
    uint16_t vendor_id_;
    uint16_t product_id_;
    std::string vendor_id_str_;
    std::string product_id_str_;
    
    // Camera finding 
    bool device_found_;
    uint8_t bus_num_;
    uint8_t address_;
    
    // USB device finding
    bool usb_device_found_;
    //libusb_context *m_context;
    //libusb_device_handle *m_handle;
    
    UsbSymlink(
      const char* symlink,
      uint16_t vendor_id,
      uint16_t product_id
    ) {
      symlink_ = std::string(symlink);
      
      vendor_id_ = vendor_id;
      product_id_ = product_id;
      std::stringstream vss; vss << std::hex << vendor_id_;
      std::stringstream pss; pss << std::hex << product_id_;
      vendor_id_str_ = vss.str();
      product_id_str_ = pss.str();
      
      device_found_ = false;
      bus_num_ = 0;
      address_ = 0;
      
      usb_device_found_ = false;
      //m_context = NULL;
      //m_handle = NULL;
      
      std::cout << "Created (uninitiated) camera with: " << std::endl;
      std::cout << "  symlink = " << symlink_ << std::endl;
      std::cout << "  vendor id = " << vendor_id_str_ << std::endl;
      std::cout << "  product id = " << product_id_str_ << std::endl;
      
      if (!FindCameraSymlink()) {
        std::cout << "Could not find camera symlink. Check /dev directory, udev rules and if camera is connected.";
      }
      
    }
    
    virtual ~UsbSymlink() {}
    
    
    // Find the Symlink using UDev library
    bool FindCameraSymlink() {
	    struct udev *udev;
	    struct udev_enumerate *enumerate;
	    struct udev_list_entry *devices, *dev_list_entry;
	    struct udev_device *dev;
      unsigned int bus_num, address;
      bus_num = 0;
      address = 0;
	
	    /* Create the udev object */
	    udev = udev_new();
	    if (!udev) {
		    std::cout << "Can't create udev" << std::endl;
		    return false;
	    }
	
	    /* Create a list of the devices in the 'hidraw' subsystem. */
	    enumerate = udev_enumerate_new(udev);
	    udev_enumerate_add_match_sysattr(enumerate, "idVendor", vendor_id_str_.c_str());
	    udev_enumerate_add_match_sysattr(enumerate, "idProduct", product_id_str_.c_str());
	    udev_enumerate_scan_devices(enumerate);
	    devices = udev_enumerate_get_list_entry(enumerate);
	    /* For each item enumerated, print out its information.
	       udev_list_entry_foreach is a macro which expands to
	       a loop. The loop will be executed for each member in
	       devices, setting dev_list_entry to a list entry
	       which contains the device's path in /sys. */
	    udev_list_entry_foreach(dev_list_entry, devices) {
		    const char *path;
		    
		    if (!device_found_) {
		
		      /* Get the filename of the /sys entry for the device
		         and create a udev_device object (dev) representing it */
		      path = udev_list_entry_get_name(dev_list_entry);
		      dev = udev_device_new_from_syspath(udev, path);
		      
		      // Lookup symlinks for the device
		      struct udev_list_entry *list = udev_device_get_devlinks_list_entry(dev);
          while (list) {
            std::string dev_symlink(udev_list_entry_get_name(list));
            std::cout << "Looking at: " << dev_symlink << std::endl;
            device_found_ = (dev_symlink == symlink_);
            list = udev_list_entry_get_next(list);
          }
          
          if (device_found_) {
            std::cout << "Found the Camera!" << std::endl;
		        /* usb_device_get_devnode() returns the path to the device node
		           itself in /dev. */
		        std::cout << "Device Node Path: " << udev_device_get_devnode(dev) << std::endl;
		        std::cout << "Device Dev Path: " << udev_device_get_devpath(dev) << std::endl;
		        std::cout << "Device Sys Path: " << udev_device_get_syspath(dev) << std::endl;

		        /* From here, we can call get_sysattr_value() for each file
		           in the device's /sys entry. The strings passed into these
		           functions (idProduct, idVendor, serial, etc.) correspond
		           directly to the files in the directory which represents
		           the USB device. Note that USB strings are Unicode, UCS2
		           encoded, but the strings returned from
		           udev_device_get_sysattr_value() are UTF-8 encoded. */
		        std::cout << "  VID/PID: "
		                << udev_device_get_sysattr_value(dev,"idVendor") << ", "
		                << udev_device_get_sysattr_value(dev, "idProduct") << std::endl;
		        std::cout << "  "
		                << udev_device_get_sysattr_value(dev,"manufacturer") << ", "
		                << udev_device_get_sysattr_value(dev,"product") << std::endl;
		        std::cout << "  serial: " 
		                << udev_device_get_sysattr_value(dev, "serial");		        
		        std::string bus_num_str(udev_device_get_sysattr_value(dev, "busnum"));
		        std::string address_str(udev_device_get_sysattr_value(dev, "devnum"));
		        std::cout << "  busnum: " << bus_num_str << std::endl;
		        std::cout << "  devnum: " << address_str << std::endl;
		        
		        bus_num = (unsigned int)std::atoi(bus_num_str.c_str());
		        address = (unsigned int)std::atoi(address_str.c_str());
		                 
		        /*struct udev_list_entry *properties_list = udev_device_get_properties_list_entry(dev);
            while (properties_list) {
              std::cout << "  property " << udev_list_entry_get_name(properties_list)
                      << " " << udev_list_entry_get_value(properties_list) << std::endl;
              properties_list = udev_list_entry_get_next(properties_list);
            }

		        struct udev_list_entry *tags_list = udev_device_get_tags_list_entry(dev);
            while (tags_list) {
              std::cout << "  tag name = " << udev_list_entry_get_name(tags_list);
              tags_list = udev_list_entry_get_next(tags_list) << std::endl;
            }*/
          }
          
		      udev_device_unref(dev);
		    } // device found
	    }
	    /* Free the enumerator object */
	    udev_enumerate_unref(enumerate);

	    udev_unref(udev);
	    
	    if (device_found_) {
	      bus_num_ = bus_num;
	      address_ = address;
  	    std::cout << "Found camera " << symlink_ << " at bus,address " << int(bus_num_) << "," << int(address_) << std::endl;
	    } else {
	      std::cout << "Could not find camera " << symlink_ << std::endl;
	    }
	    
      return true;
    } // FindCameraSymlink
    
    int GetUsbDeviceHandle(
      libusb_context *m_context,
      libusb_device_handle **m_handle
    ) {
    
      libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
	    int r; //for return values
	    ssize_t cnt; //holding number of devices in list
	    //r = libusb_init(&m_context); //initialize a library session
	    //if (r < 0) {
		  //  std::cout << "Init Error " << r << std::endl; //there was an error
			//	return false;
	    //}
	    //libusb_set_debug(m_context, 3); //set verbosity level to 3, as suggested in the documentation
	    cnt = libusb_get_device_list(m_context, &devs); //get the list of devices
	    if (cnt < 0) {
		    std::cout << "Get Device Error" << std::endl; //there was an error
	    }
	    std::cout << cnt << " Devices in list." << std::endl; //print total number of usb devices
	    for (ssize_t i=0; i<cnt; i++) {
	    
	      if (!usb_device_found_) {
	        
	        libusb_device_descriptor desc;
	        int r = libusb_get_device_descriptor(devs[i], &desc);
	        if (r < 0) {
		        std::cout << "Failed to get device descriptor" << std::endl;
		        continue;
	        }
	        
	        if (desc.idVendor == vendor_id_ && desc.idProduct == product_id_) {
	          uint8_t bus_num = libusb_get_bus_number(devs[i]);
	          uint8_t address = libusb_get_device_address(devs[i]);
	          
	          if (bus_num == bus_num_ && address == address_) {
	            usb_device_found_ = true;
	            std::cout << "Found USB device for camera" << std::endl;
	            std::cout << "Bus, address = " << int(bus_num) << " " << int(address) << std::endl;
	            std::cout << "Number of possible configurations: "<<(int)desc.bNumConfigurations<<"  "
	              <<"Device Class: "<<(int)desc.bDeviceClass<<"  "
	              <<"VendorID: "<<desc.idVendor<<"  "
	              <<"ProductID: "<<desc.idProduct << std::endl;
	              
	            std::cout << "Getting a handle to the camera" << std::endl;
	            int r_open = libusb_open(devs[i], m_handle);
	            if (r_open == 0) {
	              std::cout << "USB device handle was received successfuly" << std::endl;
	            } else {
	              std::cout << "USB device handle was not recieved." << std::endl;
	              std::cout << "Error code = " << r_open << std::endl;
	            }
	            
	          } // if bus number and address match
          	
	        } // if vid,pid match
	      
	      } // if not found yet	      
	      
	    }
	    libusb_free_device_list(devs, 1); //free the list, unref the devices in it
    
      if (usb_device_found_) {
        std::cout << "USB Camera was found successfully" << std::endl;
      } else {
        std::cout << "Could not find USB camera" << std::endl;
        return 1;
      }
    
      return 0;
    }  // GetUsbDeviceHandle
    
    
    // Open 
    
  }; // UsbSymlink



#endif

