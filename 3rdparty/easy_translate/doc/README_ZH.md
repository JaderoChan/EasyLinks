# EasyTranslate

**简体中文** | [English](README_EN.md)

## 介绍

这是一个能够帮助你快速完成GUI程序中UI文本翻译的mini库，使用C++11编写。

## 特点

- 小且简单：这是一个Header-Only库，这意味着你可以很轻松的将它应用在任何C++项目中。
- 语言文件透明，无需编译：译文文件使用json格式文本文件存储，这意味着它无需编译，易于修改，且对人类阅读友好。
- 自动提取**译文ID**：提供接口与python脚本帮助开发者提取代码中用到的**译文ID**（通过**译文ID**获取译文）。

## 缺点

- 无译文文件动态载入功能，在切换语言时会将目标语言的所有译文载入至内存。
- 严格依赖于文件系统，这意味着本库只接受文件形式的译文文件。

## 依赖

[Nlohmann/Json](https://github.com/nlohmann/json)

## 大纲

### Languages

此类存储了语言ID与译文文件路径的映射。

其基本结构是`{Language ID (key, string) : Translations filepath (value, string)}`对。其中`Language ID`表示语言ID，如`EN`、`ZH`等，而`Translations filepath`表示语言ID对应的译文文件路径。

### Translations

此类存储了译文ID与译文的映射。

其基本结构是`{Text ID (key, string) : Translation (value, string)}`对，其中`Text ID`表示译文ID，而`Translation`表示译文ID对应的译文。

## 标准使用流程

1. 通过`setLanguages`载入一个语言列表文件（内部会将文件解析为`Language`类，若解析失败会设置一个空`Language`），这样便得到了语言ID与译文文件路径的映射。
2. 通过`changeLanguage`改变当前语言ID，这样便能通过语言ID将目标译文文件载入。
3. 通过`EASYTR`宏对程序中的译文ID进行包含（或者调用`translate`）以获取当前语言下指定译文ID的对应译文。

## 提取译文ID

大多数时候，手动提取译文ID是十分繁琐的。

故可以在程序主循环结束后调用`updateTranslationsFiles`来更新译文ID，这会在译文文件中新增一个键值对，其中键被设置为新发现的译文ID，值（译文）被设置为空字符串，并对译文ID重排序。

使用`updateTranslationsFiles`时必须定义`EASY_TRANSLATE_DUMP_TEXTID`宏，否则不做任何事情。

在定义`EASY_TRANSLATE_DUMP_TEXTID`宏时，内部会对每一个读取到的译文ID进行存储，以便在调用`updateTranslationsFiles`时对译文文件进行更新，故此时会消耗额外性能。正确的做法应该是在需要提取译文ID时才定义`EASY_TRANSLATE_DUMP_TEXTID`宏。

## 示例

参见`example/base_example`，其提供了一个在Qt框架下实现的示例程序项目。
