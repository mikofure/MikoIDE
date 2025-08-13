import type { Dictionary } from './en';

export const dict: Dictionary = {
  // Menu items
  menu: {
    file: {
      title: "ファイル",
      newFile: "新しいファイル",
      openFile: "ファイルを開く...",
      openFolder: "フォルダを開く...",
      save: "保存",
      saveAs: "名前を付けて保存...",
      exit: "終了"
    },
    edit: {
      title: "編集",
      undo: "元に戻す",
      redo: "やり直し",
      cut: "切り取り",
      copy: "コピー",
      paste: "貼り付け",
      selectAll: "すべて選択",
      find: "検索",
      findNext: "次を検索",
      findPrevious: "前を検索",
      replace: "置換",
      goToLine: "行に移動",
      formatDocument: "ドキュメントのフォーマット",
      toggleLineComment: "行コメントの切り替え",
      toggleBlockComment: "ブロックコメントの切り替え",
      duplicateLine: "行の複製",
      moveLineUp: "行を上に移動",
      moveLineDown: "行を下に移動",
      deleteLine: "行の削除",
      indentLines: "インデントを増やす",
      outdentLines: "インデントを減らす",
      toggleWordWrap: "ワードラップの切り替え",
      foldAll: "すべて折りたたむ",
      unfoldAll: "すべて展開"
    },
    view: {
      title: "表示",
      toggleSidebar: "サイドバーの切り替え",
      toggleBottomPanel: "下部パネルの切り替え",
      toggleRightPanel: "右パネルの切り替え"
    },
    help: {
      title: "ヘルプ",
      about: "について"
    }
  },
  // UI elements
  ui: {
    welcome: {
      title: "MikoIDE へようこそ",
      subtitle: "モダンなコードエディタ",
      openFile: "ファイルを開く",
      openFolder: "フォルダを開く",
      newFile: "新しいファイル"
    },
    statusBar: {
      line: "行",
      column: "列",
      selection: "選択",
      language: "言語",
      words: "単語",
      chars: "文字",
      debug: "デバッグ",
      lsp: "LSP",
      prettier: "Prettier"
    },
    explorer: {
      title: "エクスプローラー",
      noFolderOpened: "フォルダが開かれていません",
      openFolder: "フォルダを開く",
      tryAgain: "再試行",
      loadingFolder: "フォルダを読み込み中...",
      folderNotSupported: "ウェブブラウザではフォルダを開くことができません",
      failedToLoadDirectory: "ディレクトリの読み込みに失敗しました",
      failedToLoadFolder: "フォルダの読み込みに失敗しました",
      folderNotSupportedBrowser: "ウェブブラウザモードではフォルダを開くことができません",
      failedToOpenFolder: "フォルダを開くことができませんでした",
      folderNotTrusted: "フォルダアクセスが信頼されませんでした"
    },
    //@ts-expect-error
    welcome: {
      title: "MikoIDE",
      subtitle: "最先端のウェブ技術とネイティブパフォーマンスで構築された、モダンなクロスプラットフォーム統合開発環境（IDE）です。",
      newFile: "新しいファイル",
      openFolder: "フォルダを開く",
      quickActions: "クイックアクション",
      cloneRepository: "Gitリポジトリをクローン",
      connectRemote: "リモートに接続",
      newWorkspace: "新しいワークスペース",
      openTerminal: "ターミナルを開く",
      recentProjects: "最近のプロジェクト",
      loadingRecentProjects: "最近のプロジェクトを読み込み中...",
      noRecentProjects: "最近のプロジェクトはありません",
      refreshRecentProjects: "最近のプロジェクトを更新...",
      learningResources: "学習リソース",
      getStarted: "MikoIDEを始める",
      getStartedDesc: "基本を学んでコーディングを開始",
      accessibilityFeatures: "アクセシビリティ機能",
      accessibilityDesc: "アクセシビリティのためのツールとショートカット",
      powershellGuide: "PowerShellガイド",
      learnFundamentals: "基礎を学ぶ",
      gitlensGuide: "GitLensガイド",
      browseAllTutorials: "すべてのチュートリアルを閲覧...",
      footerText: "MikoIDEへようこそ - あなたのアクセシブルな開発環境",
      documentation: "ドキュメント",
      community: "コミュニティ",
      support: "サポート",
      badgeNew: "新着",
      badgeUpdated: "更新済み"
    },
    tabs: {
      untitled: "無題",
      close: "閉じる",
      closeAll: "すべて閉じる",
      closeOthers: "他を閉じる"
    }
  },
  // Common actions
  actions: {
    ok: "OK",
    cancel: "キャンセル",
    save: "保存",
    open: "開く",
    close: "閉じる",
    delete: "削除",
    rename: "名前の変更",
    create: "作成",
    edit: "編集"
  },
  // Messages
  messages: {
    fileNotFound: "ファイルが見つかりません",
    saveSuccess: "ファイルが正常に保存されました",
    saveError: "ファイルの保存中にエラーが発生しました",
    openError: "ファイルを開く際にエラーが発生しました",
    unsavedChanges: "保存されていない変更があります。保存しますか？"
  }
};