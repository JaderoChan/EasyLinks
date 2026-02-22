# EasyTranslate

[简体中文](README_ZH.md) | **Endlish**

## Introduction

This is a mini library that can help you quickly complete the UI text translation of GUI application, written in C++11.

## Features

- Small and Simple: This is a header-only library, meaning you can easily integrate it into any C++ project.
- Transparent Language Files, No Compilation Required: Translation files are stored in JSON format, which means they do not require compilation, are easy to modify, and are human-readable.
- Automatic Extraction of **Text IDs**: Provides interfaces and Python scripts to help developers extract **Text IDs** used in the code (translations are retrieved via **Text IDs**).

## Disadvantages

- No dynamic loading of translation files; when switching languages, all translations of the target language are loaded into memory.
- Strictly dependent on the file system, meaning this library only accepts translation files in file form.

## Dependence

[Nlohmann/Json](https://github.com/nlohmann/json)

## Overview

### Languages

This class stores the mapping between language IDs and translation file paths.

Its basic structure is a `{Language ID (key, string) : Translations filepath (value, string)}` pair. Here, `Language ID` represents the language ID, such as `EN`, `ZH`, etc., and `Translations filepath` represents the file path of the translation file corresponding to the language ID.

### Translations

This class stores the mapping between text IDs and translations.

Its basic structure is a `{Text ID (key, string) : Translation (value, string)}` pair, where `Text ID` represents the text ID, and `Translation` represents the translation corresponding to the text ID.

## Standard Usage Process

1. Use `setLanguages` to load a language list file (internally, the file is parsed into a `Language` class; if parsing fails, an empty `Language` is set). This gives you the mapping between language IDs and translation file paths.
2. Use `changeLanguage` to change the current language ID, which loads the target translation file via the language ID.
3. Use the `EASYTR` macro to wrap text IDs in the program (or call `translate`) to retrieve the translation corresponding to the specified text ID in the current language.

## Extracting Text IDs

Most of the time, manually extracting text IDs is tedious.

Therefore, you can call `updateTranslationsFiles` after the main program loop to update text IDs. This will add a new key-value pair in the translation file, where the key is set to the newly discovered text ID, the value (translation) is set to an empty string, and the text IDs are reordered.

When using `updateTranslationsFiles`, the `EASY_TRANSLATE_DUMP_TEXTID` macro must be defined; otherwise, nothing will be done.

When the `EASY_TRANSLATE_DUMP_TEXTID` macro is defined, every retrieved text ID is stored internally so that the translation files can be updated when `updateTranslationsFiles` is called. This consumes additional performance. The correct approach is to define the `EASY_TRANSLATE_DUMP_TEXTID` macro only when text ID extraction is needed.

## Example

Refer to `example/base_example`, which provides an example program project implemented under the Qt framework.
