# YADownloader

Yet another simple download(manager) library written in pure Qt5.

### Basic usage

```
....

YADownloader::DLTaskAccessMgr *m_downloadMgr = new new DLTaskAccessMgr(this);
YADownloader::DLRequest req(url, savePath, saveName);
YADownloader::DLTask *m_task = m_downloadMgr->get(req);
connect(m_task, &DLTask::statusChanged, [&](const QString &uuid, DLTask::TaskStatus status) {
.....
};
connect(m_task, &DLTask::taskInfoChanged, [&](const QString &uuid, const DLTaskInfo &info) {
....
};
...

m_task->deleteLater();
m_task = nullptr;
```


### For integrating with qml
#### [BNetDisk](https://github.com/SunRain/BNetDisk)

[main.cpp](https://github.com/SunRain/BNetDisk/blob/master/src/main.cpp) 
[BDiskDownloadDelegate.cpp](https://github.com/SunRain/BNetDisk/blob/master/src/BDiskDownloadDelegate.cpp) 
[DownloadStore.qml](https://github.com/SunRain/BNetDisk/blob/master/qml/QuickFlux/Stores/DownloadStore.qml)
[DownloadingTaskView.qml](https://github.com/SunRain/BNetDisk/blob/master/qml/UI/DownloadingTaskView.qml)
