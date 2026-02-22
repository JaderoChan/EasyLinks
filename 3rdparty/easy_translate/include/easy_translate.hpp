// The "easy translate" library for C++.
//
// Repository: https://github.com/JaderoChan/easy_translate
// Contact email: c_dl_cn@outlook.com

// MIT License
//
// Copyright (c) 2024 頔珞JaderoChan

#ifndef EASY_TRANSLATE_HPP
#define EASY_TRANSLATE_HPP

#include <cstddef>              // size_t
#include <string>               // string
#include <vector>               // vector
#include <set>                  // set
#include <map>                  // map
#include <fstream>              // ifstream, ofstream

#include <nlohmann/json.hpp>    // json

/// @def EASY_TRANSLATE_UPDATE_TRANSLATIONS_FILES
/// @brief Define this macro to enable the `easytr::updateTranslationsFiles()` function.
/// @note When this macro is defined, the `easytr::TranslateManager::translate()` function will store
/// all `Translation ID`s in memory for potential updates to `Translations files`.

// Translate function
//   - Usage: EASYTR("Translation ID")
//   - Returns the `Translation text` for the given `Translation ID` in the current language.
//   - If the `Translation ID` does not exist in the current language, returns the `Translation ID` itself.
#define EASYTR(x) easytr::translate(x)

// Below is an example directory structure and content format for
// the `Languages file` and `Translations files`:
//
// languages.json (Languages file)
//   - en_US (Language ID) : en_US.json (Translations filepath)
//   - zh_CN (Language ID) : zh_CN.json (Translations filepath)
//   - ...
//   - ja_JP (Language ID) : ja_JP.json (Translations filepath)
//   - fr_FR (Language ID) : fr_FR.json (Translations filepath)
//
// en_US.json (Translations file)
//   - App.Title  (Translation ID) : Easy Translation (Translation text)
//   - App.Author (Translation ID) : JaderoChan       (Translation text)
//   - ...
//
// zh_CN.json
//   - App.Title  (Translation ID) : 轻松翻译 (Translation text)
//   - App.Author (Translation ID) : 頔珞     (Translation text)
//   - ...

// Languages
//   - Language ID : Translations filepath
//   - ...
//
// Translations
//   - Translation ID : Translation text
//   - ...

// Note:
// The `Languages file` (e.g., languages.json) and `Translations files` (e.g., en_US.json, zh_CN.json)
// should be saved using UTF-8 encoding.

namespace easytr
{

class Languages
{
    friend class TranslateManager;

public:
    Languages() = default;

    Languages(const std::map<std::string, std::string>& langs) : languages_(langs) {}

    /// @brief Load `Languages` from a JSON string.
    /// @note If the JSON is invalid, the `Languages` object will be empty.
    static Languages fromJson(const std::string& json)
    {
        using Json = nlohmann::json;

        Json j = Json::parse(json, nullptr, false, true);
        if (j.is_discarded())
            return Languages();

        std::map<std::string, std::string> list;
        for (const auto& var : j.items())
            list.insert({var.key(), var.value()});

        return Languages(list);
    }

    /// @brief Load `Languages` from a JSON file.
    /// @note If the JSON is invalid, the `Languages` object will be empty.
    static Languages fromFile(const std::string& filepath)
    {
        using Json = nlohmann::json;

        std::ifstream ifs(filepath);
        if (!ifs.is_open())
            return Languages();

        Json j = Json::parse(ifs, nullptr, false, true);
        if (j.is_discarded())
        {
            ifs.close();
            return Languages();
        }

        std::map<std::string, std::string> list;
        ifs.close();
        for (const auto& var : j.items())
            list.insert({var.key(), var.value()});

        return Languages(list);
    }

    /// @brief Get the JSON representation as a string.
    std::string toJson() const
    {
        nlohmann::json j;
        for (const auto& var : languages_)
            j[var.first] = var.second;
        return j.dump(4);
    }

    /// @brief Write the `Languages` to a JSON file.
    /// @return Returns false if writing fails, otherwise returns true.
    bool toFile(const std::string& filepath = "languages.json") const
    {
        std::ofstream ofs(filepath);
        if (!ofs.is_open())
            return false;
        ofs << toJson();
        ofs.close();
        return true;
    }

    /// @brief Get the `Translations filepath` for the given `Language ID`.
    const char* at(const std::string& languageId) const
    { return languages_.at(languageId).c_str(); }

    /// @brief Get the number of `Language ID`s.
    size_t count() const { return languages_.size(); }

    /// @brief Check if there are no `Language ID`s.
    bool empty() const { return count() == 0; }

    /// @brief Check if the given `Language ID` exists.
    bool has(const std::string& languageId) const
    { return languages_.find(languageId) != languages_.end(); }

    /// @brief Get all `Language ID`s.
    std::vector<std::string> getIds() const
    {
        std::vector<std::string> ids;
        for (const auto& var : languages_)
            ids.push_back(var.first);
        return ids;
    }

    /// @brief Add a pair of `Language ID` and `Translations filepath`.
    /// @note If the `Language ID` already exists, no action is taken.
    void add(const std::string& languageId, const std::string& translationsFilepath)
    {
        if (!has(languageId))
            languages_.insert({languageId, translationsFilepath});
    }

    /// @brief Remove a `Language ID` and its corresponding `Translations filepath`.
    void remove(const std::string& languageId)
    {
        if (has(languageId))
            languages_.erase(languageId);
    }

    /// @brief Remove all `Language ID`s and their corresponding `Translations filepath`s.
    void clear() { languages_.clear(); }

private:
    // {Language ID : Translations filepath}
    std::map<std::string, std::string> languages_;
};

class Translations
{
    friend class TranslateManager;
public:
    Translations() = default;

    Translations(const std::map<std::string, std::string>& trans) : translations_(trans) {}

    /// @brief Load `Translations` from a JSON string.
    /// @note If the JSON is invalid, the `Translations` object will be empty.
    static Translations fromJson(const std::string& json)
    {
        using Json = nlohmann::json;

        Json j = Json::parse(json, nullptr, false, true);
        if (j.is_discarded())
            return Translations();

        std::map<std::string, std::string> list;
        for (const auto& var : j.items())
            list.insert({var.key(), var.value()});

        return Translations(list);
    }

    /// @brief Load `Translations` from a JSON file.
    /// @note If the JSON is invalid, the `Translations` object will be empty.
    static Translations fromFile(const std::string& filepath)
    {
        using Json = nlohmann::json;

        std::ifstream ifs(filepath);
        if (!ifs.is_open())
            return Translations();

        Json j = Json::parse(ifs, nullptr, false, true);
        if (j.is_discarded())
        {
            ifs.close();
            return Translations();
        }

        std::map<std::string, std::string> list;
        ifs.close();
        for (const auto& var : j.items())
            list.insert({var.key(), var.value()});

        return Translations(list);
    }

    /// @brief Get the JSON representation as a string.
    std::string toJson() const
    {
        nlohmann::json j;
        for (const auto& var : translations_)
            j[var.first] = var.second;
        return j.dump(4);
    }

    /// @brief Write the `Translations` to a JSON file.
    /// @return Returns false if writing fails, otherwise returns true.
    bool toFile(const std::string& filepath) const
    {
        std::ofstream ofs(filepath);
        if (!ofs.is_open())
            return false;
        ofs << toJson();
        ofs.close();
        return true;
    }

    /// @brief Get the `Translation text` for the given `Translation ID`.
    /// @note If the `Translation ID` does not exist, returns the `Translation ID` itself.
    const char* at(const std::string& tranId) const
    {
        if (!has(tranId))
            return tranId.c_str();
        return translations_.at(tranId).c_str();
    }

    /// @brief Get the number of `Translation ID`s.
    size_t count() const { return translations_.size(); }

    /// @brief Check if there are no `Translation ID`s.
    bool empty() const { return count() == 0; }

    /// @brief Check if the given `Translation ID` exists.
    bool has(const std::string& tranId) const
    { return translations_.find(tranId) != translations_.end(); }

    /// @brief Get all `Translation ID`s.
    std::vector<std::string> getIds() const
    {
        std::vector<std::string> ids;
        for (const auto& var : translations_)
            ids.push_back(var.first);
        return ids;
    }

    /// @brief Add a pair of `Translation ID` and `Translation text`.
    /// @note If the `Translation ID` already exists, no action is taken.
    void add(const std::string& tranId, const std::string& translation)
    {
        if (!has(tranId))
            translations_.insert({tranId, translation});
    }

    /// @brief Remove a `Translation ID` and its corresponding `Translation text`.
    void remove(const std::string& tranId)
    {
        if (has(tranId))
            translations_.erase(tranId);
    }

    /// @brief Remove all `Translation ID`s and their corresponding `Translation text`s.
    void clear() { translations_.clear(); }

private:
    // {Translation ID : Translation text}
    std::map<std::string, std::string> translations_;
};

// Singleton class
class TranslateManager
{
public:
    static TranslateManager& getInstance()
    {
        static TranslateManager instance;
        return instance;
    }

    /// @brief Get the `Translation text` for the given `Translation ID` in the current language.
    /// @note If the `Translation ID` does not exist in the current language, returns the `Translation ID` itself.
#ifndef EASY_TRANSLATE_UPDATE_TRANSLATIONS_FILES
    const char* translate(const std::string& tranId) const
    {
        return translations_.at(tranId);
    }
#else
    const char* translate(const std::string& tranId)
    {
        tranIds_.insert(tranId);
        return translations_.at(tranId);
    }
#endif // EASY_TRANSLATE_UPDATE_TRANSLATIONS_FILES

    /// @brief Set the `Languages` and reset the current language.
    void setLanguages(const Languages& languages) { languages_ = languages; currentLanguage_.clear(); }

    /// @brief Get the `Language ID` of the current language.
    const char* currentLanguage() const { return currentLanguage_.c_str(); }

    /// @brief Set the current language by `Language ID`.
    /// @return Returns true if successful, otherwise returns false.
    bool setCurrentLanguage(const std::string& languageId)
    {
        if (!hasLanguage(languageId))
            return false;

    #ifdef EASY_TRANSLATE_UPDATE_TRANSLATIONS_FILES
        bool isFirst = currentLanguage_.empty();
    #endif // EASY_TRANSLATE_UPDATE_TRANSLATIONS_FILES
        currentLanguage_ = languageId;
        translations_ = Translations::fromFile(languages_.at(languageId));

    #ifdef EASY_TRANSLATE_UPDATE_TRANSLATIONS_FILES
        if (isFirst)
        {
            for (const auto& var : translations_.translations_)
                tranIds_.insert(var.first);
        }
    #endif // EASY_TRANSLATE_UPDATE_TRANSLATIONS_FILES

        return true;
    }

    const Languages& languages() const { return languages_; }

    const Translations& translations() const { return translations_; }

    /// @brief Get the number of `Language ID`s.
    size_t languageCount() const { return languages_.count(); }

    /// @brief Get the number of `Translation ID`s in the current language.
    size_t translationCount() const { return translations_.count(); }

    /// @brief Check if the given `Language ID` exists.
    bool hasLanguage(const std::string& languageId) const { return languages_.has(languageId); }

    /// @brief Check if the given `Translation ID` exists in the current language.
    bool hasTranslation(const std::string& tranId) const { return translations_.has(tranId); }

    /// @brief Update all `Translations files` (add new `Translation ID`s with empty `Translation text`).
    /// @return The number of files updated.
    /// @note New `Translation ID`s are collected from all calls to `translate()` in the program.
    /// @note This function helps to easily obtain all `Translation ID`s that need translation.
    /// @attention Call this function after all `translate()` calls to ensure a complete `Translation ID` list.
    /// @attention This function has no effect when the macro `EASY_TRANSLATE_UPDATE_TRANSLATIONS_FILES` is undefined.
    size_t updateTranslationsFiles() const
    {
    #ifndef EASY_TRANSLATE_UPDATE_TRANSLATIONS_FILES
        return 0;
    #else
        using Json = nlohmann::json;

        size_t updated = 0;
        for (const auto& languageId : languages_.getIds())
        {
            std::string filepath = languages_.at(languageId);
            std::ifstream ifs(filepath);
            Json j;
            if (!ifs.is_open())
            {
                for (const auto& tranId : tranIds_)
                    j[tranId] = "";
            }
            else
            {
                j = Json::parse(ifs, nullptr, false, true);
                if (j.is_discarded())
                {
                    j = Json();
                    for (const auto& tranId : tranIds_)
                        j[tranId] = "";
                }
                else
                {
                    std::map<std::string, std::string> map; // For sorting
                    for (const auto& tranId : tranIds_)
                        j.contains(tranId) ? map.insert({tranId, j[tranId]}) : map.insert({tranId, ""});

                    j.clear();
                    for (const auto& var : map)
                        j[var.first] = var.second;
                }

                ifs.close();
            }

            std::ofstream ofs(filepath);
            if (!ofs.is_open())
                continue;

            ofs << j.dump(4);
            ofs.close();
            updated++;
        }

        return updated;
    #endif // EASY_TRANSLATE_UPDATE_TRANSLATIONS_FILES
    }

private:
    TranslateManager() = default;

    ~TranslateManager() = default;

    TranslateManager(const TranslateManager&) = delete;

    TranslateManager& operator=(const TranslateManager&) = delete;

#ifdef EASY_TRANSLATE_UPDATE_TRANSLATIONS_FILES
    std::set<std::string> tranIds_;
#endif // EASY_TRANSLATE_UPDATE_TRANSLATIONS_FILES
    std::string currentLanguage_;
    Languages languages_;
    Translations translations_;
};

// Convenience functions

inline TranslateManager& getTranslateManager()
{ return TranslateManager::getInstance(); }

/// @brief Get the `Translation text` for the given `Translation ID` in the current language.
/// @note If the `Translation ID` does not exist in the current language, returns the `Translation ID` itself.
inline const char* translate(const std::string& tranId)
{ return getTranslateManager().translate(tranId); }

/// @brief Set the `Languages`.
inline void setLanguages(const Languages& langs)
{ getTranslateManager().setLanguages(langs); }

inline const char* currentLanguage()
{ return getTranslateManager().currentLanguage(); }

/// @brief Set the current language by `Language ID`.
/// @return Returns true if successful, otherwise returns false.
inline bool setCurrentLanguage(const std::string& languageId)
{ return getTranslateManager().setCurrentLanguage(languageId); }

/// @brief Get the number of `Language ID`s.
inline size_t languageCount()
{ return getTranslateManager().languageCount(); }

/// @brief Get the number of `Translation ID`s in the current language.
inline size_t translationCount()
{ return getTranslateManager().translationCount(); }

/// @brief Check if the given `Language ID` exists.
inline bool hasLanguage(const std::string& languageId)
{ return getTranslateManager().hasLanguage(languageId); }

/// @brief Check if the given `Translation ID` exists in the current language.
inline bool hasTranslation(const std::string& tranId)
{ return getTranslateManager().hasTranslation(tranId); }

inline const Languages& languages()
{ return getTranslateManager().languages(); }

inline const Translations& translations()
{ return getTranslateManager().translations(); }

/// @brief Update all `Translations files` (add new `Translation ID`s with empty `Translation text`).
/// @return The number of files updated.
/// @note New `Translation ID`s are collected from all calls to `translate()` in the program.
/// @note This function helps to easily obtain all `Translation ID`s that need translation.
/// @attention Call this function after all `translate()` calls to ensure a complete `Translation ID` list.
/// @attention This function has no effect when the macro `EASY_TRANSLATE_UPDATE_TRANSLATIONS_FILES` is undefined.
inline size_t updateTranslationsFiles()
{ return getTranslateManager().updateTranslationsFiles(); }

} // namespace easytr

#endif // !EASY_TRANSLATE_HPP
