# QtCourse

刘楷钦的 Qt 应用程序开发课程作业。主分支负责课程概览，每份作业在独立分支维护源码、运行说明和测试。

## 分支导航

| 分支 | 内容 | Qt Creator 打开的项目 |
| --- | --- | --- |
| [main](https://github.com/lkq-guji/QtCourse/tree/main) | 概览和分支导航 | 无 |
| [chapter02-about](https://github.com/lkq-guji/QtCourse/tree/chapter02-about) | 工具栏 About 按钮、个人信息弹窗 | `samp2_4App/samp2_4.pro` |
| [chapter04-tablewidget](https://github.com/lkq-guji/QtCourse/tree/chapter04-tablewidget) | 五人七列表格、本人红色粗体、籍贯关联与状态栏 | `samp4_13TableWidget/samp4_13.pro` |

## 下载和运行

只下载需要的作业：

```bash
git clone --branch chapter04-tablewidget --single-branch git@github.com:lkq-guji/QtCourse.git QtCourse-tablewidget
```

在 Qt Creator 中打开该分支的 `.pro` 文件，选择 Desktop Qt / MinGW Kit，构建后运行。两个作业在 Windows、Qt 6.11.1、MinGW 13.1.0 环境下验证。

已经克隆仓库且工作区没有未提交修改时，可以切换分支：

```bash
git fetch origin
git switch chapter02-about
```

建议不同作业使用独立目录，避免 Qt Creator 缓存和构建输出混用。

## 作业说明

第二章展示 QAction、信号与槽、资源文件和 QMessageBox。原有提交历史以及已经填写的个人信息保留在 `chapter02-about`。

第四章以提供的 `samp4_13TableWidget` 工程完成题目所称的 samp4_9 表格任务。最终名单按后续指定改为刘楷钦、刘泽、龙智森、罗宇丰、许沣睿。数据来源、实现讲解和验证结果见该分支 README。

各作业分支保存可重新构建的源码。构建目录、Qt Creator 个人配置和整份原始点名册保留在本地。主分支调整采用正常提交，保留历史，无需强制推送。
