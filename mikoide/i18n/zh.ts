import type { Dictionary } from './en';

export const dict: Dictionary = {
  // Menu items
  menu: {
    file: {
      title: "文件",
      newFile: "新建文件",
      openFile: "打开文件...",
      openFolder: "打开文件夹...",
      save: "保存",
      saveAs: "另存为...",
      exit: "退出"
    },
    edit: {
      title: "编辑",
      undo: "撤销",
      redo: "重做",
      cut: "剪切",
      copy: "复制",
      paste: "粘贴",
      selectAll: "全选",
      find: "查找",
      findNext: "查找下一个",
      findPrevious: "查找上一个",
      replace: "替换",
      goToLine: "转到行",
      formatDocument: "格式化文档",
      toggleLineComment: "切换行注释",
      toggleBlockComment: "切换块注释",
      duplicateLine: "复制行",
      moveLineUp: "向上移动行",
      moveLineDown: "向下移动行",
      deleteLine: "删除行",
      indentLines: "增加缩进",
      outdentLines: "减少缩进",
      toggleWordWrap: "切换自动换行",
      foldAll: "折叠全部",
      unfoldAll: "展开全部"
    },
    view: {
      title: "视图",
      toggleSidebar: "切换侧边栏",
      toggleBottomPanel: "切换底部面板",
      toggleRightPanel: "切换右侧面板"
    },
    help: {
      title: "帮助",
      about: "关于"
    }
  },
  // UI elements
  ui: {
    welcome: {
      title: "MikoIDE",
      subtitle: "一个现代化的跨平台集成开发环境（IDE），采用前沿网络技术构建，具备原生性能。",
      newFile: "新建文件",
      openFolder: "打开文件夹",
      quickActions: "快速操作",
      cloneRepository: "克隆 Git 仓库",
      connectRemote: "连接到远程",
      newWorkspace: "新建工作区",
      openTerminal: "打开终端",
      recentProjects: "最近项目",
      loadingRecentProjects: "正在加载最近项目...",
      noRecentProjects: "没有最近项目",
      refreshRecentProjects: "刷新最近项目...",
      learningResources: "学习资源",
      getStarted: "开始使用 MikoIDE",
      getStartedDesc: "学习基础知识并开始编码",
      accessibilityFeatures: "无障碍功能",
      accessibilityDesc: "无障碍工具和快捷键",
      powershellGuide: "PowerShell 指南",
      learnFundamentals: "学习基础知识",
      gitlensGuide: "GitLens 指南",
      browseAllTutorials: "浏览所有教程...",
      footerText: "欢迎使用 MikoIDE - 您的无障碍开发环境",
      documentation: "文档",
      community: "社区",
      support: "支持",
      badgeNew: "新",
      badgeUpdated: "已更新"
    },
    statusBar: {
      line: "行",
      column: "列",
      selection: "选择",
      language: "语言",
      words: "词",
      chars: "字符",
      debug: "调试",
      lsp: "LSP",
      prettier: "Prettier"
    },
    explorer: {
      title: "资源管理器",
      noFolderOpened: "未打开文件夹",
      openFolder: "打开文件夹",
      tryAgain: "重试",
      loadingFolder: "正在加载文件夹...",
      folderNotSupported: "网页浏览器不支持打开文件夹",
      failedToLoadDirectory: "加载目录失败",
      failedToLoadFolder: "加载文件夹失败",
      folderNotSupportedBrowser: "网页浏览器模式不支持打开文件夹",
      failedToOpenFolder: "打开文件夹失败"
    },
    tabs: {
      untitled: "未命名",
      close: "关闭",
      closeAll: "关闭全部",
      closeOthers: "关闭其他"
    }
  },
  // Common actions
  actions: {
    ok: "确定",
    cancel: "取消",
    save: "保存",
    open: "打开",
    close: "关闭",
    delete: "删除",
    rename: "重命名",
    create: "创建",
    edit: "编辑"
  },
  // Messages
  messages: {
    fileNotFound: "文件未找到",
    saveSuccess: "文件保存成功",
    saveError: "保存文件时出错",
    openError: "打开文件时出错",
    unsavedChanges: "您有未保存的更改。是否要保存？"
  }
};