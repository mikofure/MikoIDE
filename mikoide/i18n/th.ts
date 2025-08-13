import type { Dictionary } from './en';

export const dict: Dictionary = {
  // Menu items
  menu: {
    file: {
      title: "ไฟล์",
      newFile: "ไฟล์ใหม่",
      openFile: "เปิดไฟล์...",
      openFolder: "เปิดโฟลเดอร์...",
      save: "บันทึก",
      saveAs: "บันทึกเป็น...",
      exit: "ออก"
    },
    edit: {
      title: "แก้ไข",
      undo: "เลิกทำ",
      redo: "ทำซ้ำ",
      cut: "ตัด",
      copy: "คัดลอก",
      paste: "วาง",
      selectAll: "เลือกทั้งหมด",
      find: "ค้นหา",
      findNext: "ค้นหาถัดไป",
      findPrevious: "ค้นหาก่อนหน้า",
      replace: "แทนที่",
      goToLine: "ไปยังบรรทัด",
      formatDocument: "จัดรูปแบบเอกสาร",
      toggleLineComment: "สลับความคิดเห็นบรรทัด",
      toggleBlockComment: "สลับความคิดเห็นบล็อก",
      duplicateLine: "ทำซ้ำบรรทัด",
      moveLineUp: "ย้ายบรรทัดขึ้น",
      moveLineDown: "ย้ายบรรทัดลง",
      deleteLine: "ลบบรรทัด",
      indentLines: "เยื้องบรรทัด",
      outdentLines: "ลดเยื้องบรรทัด",
      toggleWordWrap: "สลับการตัดคำ",
      foldAll: "พับทั้งหมด",
      unfoldAll: "คลี่ทั้งหมด"
    },
    view: {
      title: "มุมมอง",
      toggleSidebar: "สลับแถบด้านข้าง",
      toggleBottomPanel: "สลับแผงด้านล่าง",
      toggleRightPanel: "สลับแผงด้านขวา"
    },
    help: {
      title: "ช่วยเหลือ",
      about: "เกี่ยวกับ"
    }
  },
  // UI elements
  ui: {
    welcome: {
      title: "MikoIDE",
      subtitle: "สภาพแวดล้อมการพัฒนาแบบบูรณาการ (IDE) ข้ามแพลตฟอร์มที่ทันสมัย สร้างด้วยเทคโนโลยีเว็บล้ำสมัยและประสิทธิภาพแบบเนทีฟ",
      newFile: "ไฟล์ใหม่",
      openFolder: "เปิดโฟลเดอร์",
      quickActions: "การดำเนินการด่วน",
      cloneRepository: "โคลน Git Repository",
      connectRemote: "เชื่อมต่อกับรีโมท",
      newWorkspace: "เวิร์กสเปซใหม่",
      openTerminal: "เปิดเทอร์มินัล",
      recentProjects: "โปรเจกต์ล่าสุด",
      loadingRecentProjects: "กำลังโหลดโปรเจกต์ล่าสุด...",
      noRecentProjects: "ไม่มีโปรเจกต์ล่าสุด",
      refreshRecentProjects: "รีเฟรชโปรเจกต์ล่าสุด...",
      learningResources: "แหล่งเรียนรู้",
      getStarted: "เริ่มต้นกับ MikoIDE",
      getStartedDesc: "เรียนรู้พื้นฐานและเริ่มเขียนโค้ด",
      accessibilityFeatures: "คุณสมบัติการเข้าถึง",
      accessibilityDesc: "เครื่องมือและทางลัดสำหรับการเข้าถึง",
      powershellGuide: "คู่มือ PowerShell",
      learnFundamentals: "เรียนรู้พื้นฐาน",
      gitlensGuide: "คู่มือ GitLens",
      browseAllTutorials: "เรียกดูบทช่วยสอนทั้งหมด...",
      footerText: "ยินดีต้อนรับสู่ MikoIDE - สภาพแวดล้อมการพัฒนาที่เข้าถึงได้ของคุณ",
      documentation: "เอกสาร",
      community: "ชุมชน",
      support: "การสนับสนุน",
      badgeNew: "ใหม่",
      badgeUpdated: "อัปเดตแล้ว"
    },
    statusBar: {
      line: "บรรทัด",
      column: "คอลัมน์",
      selection: "การเลือก",
      language: "ภาษา",
      words: "คำ",
      chars: "ตัวอักษร",
      debug: "ดีบัก",
      lsp: "LSP",
      prettier: "Prettier"
    },
    explorer: {
      title: "เอ็กซ์พลอเรอร์",
      noFolderOpened: "ไม่มีโฟลเดอร์เปิด",
      openFolder: "เปิดโฟลเดอร์",
      tryAgain: "ลองอีกครั้ง",
      loadingFolder: "กำลังโหลดโฟลเดอร์...",
      folderNotSupported: "ไม่รองรับการเปิดโฟลเดอร์ในเว็บเบราว์เซอร์",
      failedToLoadDirectory: "ไม่สามารถโหลดไดเรกทอรีได้",
      failedToLoadFolder: "ไม่สามารถโหลดโฟลเดอร์ได้",
      folderNotSupportedBrowser: "ไม่รองรับการเปิดโฟลเดอร์ในโหมดเว็บเบราว์เซอร์",
      failedToOpenFolder: "ไม่สามารถเปิดโฟลเดอร์ได้"
    },
    tabs: {
      untitled: "ไม่มีชื่อ",
      close: "ปิด",
      closeAll: "ปิดทั้งหมด",
      closeOthers: "ปิดอื่นๆ"
    }
  },
  // Common actions
  actions: {
    ok: "ตกลง",
    cancel: "ยกเลิก",
    save: "บันทึก",
    open: "เปิด",
    close: "ปิด",
    delete: "ลบ",
    rename: "เปลี่ยนชื่อ",
    create: "สร้าง",
    edit: "แก้ไข"
  },
  // Messages
  messages: {
    fileNotFound: "ไม่พบไฟล์",
    saveSuccess: "บันทึกไฟล์สำเร็จ",
    saveError: "เกิดข้อผิดพลาดในการบันทึกไฟล์",
    openError: "เกิดข้อผิดพลาดในการเปิดไฟล์",
    unsavedChanges: "คุณมีการเปลี่ยนแปลงที่ยังไม่ได้บันทึก ต้องการบันทึกหรือไม่?"
  }
};