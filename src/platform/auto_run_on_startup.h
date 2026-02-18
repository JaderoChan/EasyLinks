#pragma once

/**
 * @brief 检查程序是否是开机自启动的。
 */
bool isAutoRunOnStartUp();

/**
 * @brief 设置程序是否开机自启动。
 * @return 如果设置成功返回true，否则返回false。
 */
bool setAutoRunOnStartUp(bool enable);
