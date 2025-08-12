#ifndef UsbMscDriver_h
#define UsbMscDriver_h
#include "SdFat.h"

// If the next include fails, install the USB_HOST_SHIELD library.
#include <masstorage.h>
//------------------------------------------------------------------------------
/** Simple USB init function.
 * \param[in] usb USB host object to be initialized.
 * \return true for success or false for failure.
 */
bool initUSB(USB* usb);
//------------------------------------------------------------------------------
/** Print debug messages if USB_FAT_DBG_MODE is nonzero */
#define USB_FAT_DBG_MODE 1
//------------------------------------------------------------------------------
/** Maximum time to initialize the USB bus */
#define TIMEOUT_MILLIS 400000L
class UsbMscDriver : public FsBlockDeviceInterface {
 public:
  UsbMscDriver(BulkOnly* bulk) : m_lun(0), m_bulk(bulk) {}
  bool isBusy() { return false; }
  bool readSector(uint32_t sector, uint8_t* dst) {
    uint8_t rc = m_bulk->Read(m_lun, sector, 512, 1, dst);
    return 0 == rc;
  }
  uint32_t sectorCount() { return m_bulk->GetCapacity(m_lun); }
  bool syncDevice() { return true; }
  bool writeSector(uint32_t sector, const uint8_t* src) {
    return 0 == m_bulk->Write(m_lun, sector, 512, 1, src);
  }

  bool readSectors(uint32_t sector, uint8_t* dst, size_t ns) {
    return 0 == m_bulk->Read(m_lun, sector, 512, ns, dst);
  }
  bool writeSectors(uint32_t sector, const uint8_t* src, size_t ns) {
    return 0 == m_bulk->Write(m_lun, sector, 512, ns, src);
  }

 private:
  uint8_t m_lun;
  BulkOnly* m_bulk;
};
#endif  // UsbMscDriver_h
