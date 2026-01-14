# TODO

- 增加功能：Replace冲突处理策略的目标文件重命名可由用户决定规则。
- 增加功能：用户可选，当有失败项时操作完成后是否关闭对话框。
- 完成功能：ConflictDecisionDialog，“跳过相同日期与大小”可选框逻辑。
- ~~完成功能：ConflictDecisionDialog，表头可选框与表头打开外部链接逻辑。~~
- 完成功能：ConflictDecisionDialog，增加CheckBox模拟表头。
- 完成功能：FileLinkWorker::isOnSameDriver，其他平台的逻辑实现。
- 优化功能：增加Windows平台下长路径支持。
- 优化功能：FileLinkWorker的文件收集与文件处理异步执行，降低内存使用量（不需要存储所有收集到的条目列表然后再处理）。
