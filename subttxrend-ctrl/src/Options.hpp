/*****************************************************************************
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2021 Liberty Global Service B.V.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*****************************************************************************/


#ifndef SUBTTXREND_APP_OPTIONS_HPP_
#define SUBTTXREND_APP_OPTIONS_HPP_

#include <string>
#include <vector>
#include <map>

namespace subttxrend
{
namespace app
{

/**
 * Command line options processing helper.
 */
class Options
{
public:
    /** Option key. */
    enum class Key
    {
        /** Help (usage information). */
        HELP,
        /** Main context unix socket path. */
        MAIN_SOCKET_PATH,
        /** Config file path. */
        CONFIG_FILE_PATH
    };

    /**
     * Constructor.
     */
    Options(int argc,
            char* argv[]);

    /**
     * Destructor.
     */
    ~Options() = default;

    /**
     * Checks if options are valid.
     *
     * @retval true
     *      Options valid.
     * @retval false
     *      Options not valid.
     */
    bool isValid() const;

    /**
     * Prints usage information to standard output.
     */
    void printUsage() const;

    /**
     * Checks if separate option was defined.
     *
     * @retval true
     *      Separate option was defined.
     * @retval false
     *      Separate option was not defined.
     */
    bool hasSeparate() const;

    /**
     * Returns "separate" option specified.
     *
     * @return
     *      Separate option defined.
     *
     * @note Valid only when hasSeparate() == true.
     */
    Key getSeparate() const;

    /**
     * Returns option value.
     *
     * @param key
     *      Option key.
     *
     * @return
     *      Option value if available, defautl value otherwise.
     */
    std::string getOptionValue(Key key) const;

private:
    /** Option entry mode. */
    enum class Mode
    {
        /** Option shall be specified separately (alone). */
        SEPARATE,
        /** Configuration option - required. */
        REQUIRED,
        /** Configuration option - optional. */
        OPTIONAL
    };

    /** Option entry. */
    struct Entry
    {
        /** Option key. */
        const Key m_key;
        /** Long name. */
        const std::string m_longName;
        /** Short name. */
        const std::string m_shortName;
        /** Description. */
        const std::string m_description;
        /** Mode. */
        const Mode m_mode;
        /** Default value. */
        const std::string m_defaultValue;

        /**
         * Constructor.
         *
         * @param key
         *      Option key.
         * @param longName
         *      Long name.
         * @param shortName
         *      Short name.
         * @param description
         *      Description.
         * @param mode
         *      Mode.
         * @param defaultValue
         *      Default value.
         */
        Entry(Key key,
              std::string longName,
              std::string shortName,
              std::string description,
              Mode mode,
              std::string defaultValue = std::string()) :
                m_key(key),
                m_longName(longName),
                m_shortName(shortName),
                m_description(description),
                m_mode(mode),
                m_defaultValue(defaultValue)
        {
            // noop
        }
    };

    /** Type of entries collection. */
    typedef std::vector<Entry> EntryCollection;

    /**
     * Adds entry.
     *
     * @param entry
     *      Entry to add.
     */
    void addEntry(const Entry& entry);

    /**
     * Finds entry.
     *
     * @param name
     *      Name of entry to find (shor tor long).
     *
     * @return
     *      Iterator to entry (may be end()).
     */
    EntryCollection::const_iterator findEntry(const std::string& name) const;

    /**
     * Parses command line options.
     *
     * @param argc
     *      Argument count.
     * @param argv
     *      Argument values.
     *
     * @retval true
     *      Options parsed successfully.
     * @retval false
     *      Options parsing failed.
     */
    bool parseOptions(int argc,
                      char* argv[]);

    /**
     * Parses single command line option.
     *
     * @param cmdValue
     *      Command line value.
     *
     * @retval true
     *      Option parsed successfully.
     * @retval false
     *      Option parsing failed.
     */
    bool parseOption(const std::string& cmdValue);

    /**
     * Validates options.
     *
     * Checks that parsed option values are valid.
     *
     * @retval true
     *      Options validates successfully.
     * @retval false
     *      Options validates failed.
     */
    bool validateOptions() const;

    /**
     * Validates "separate" options.
     *
     * Checks that parsed option values are valid.
     *
     * @retval true
     *      Options validates successfully.
     * @retval false
     *      Options validates failed.
     */
    bool validateSeparateOptions() const;

    /**
     * Validates "required" options.
     *
     * Checks that parsed option values are valid.
     *
     * @retval true
     *      Options validates successfully.
     * @retval false
     *      Options validates failed.
     */
    bool validateRequiredOptions() const;

    /**
     * Sets "separate" option if specified.
     */
    void setSeparateOption();

    /**
     * Sets default values.
     */
    void setDefaultValues();

    /**
     * Returns number of values currently set.
     *
     * @return
     *      Number of values.
     */
    std::size_t getValuesCount() const;

    /**
     * Checks if value for given entry is set.
     *
     * @param entry
     *      Entry.
     *
     * @retval true
     *      Value is set.
     * @retval false
     *      Value is not set.
     */
    bool hasValue(const Entry& entry) const;

    /**
     * Returns value for given entry.
     *
     * @param entry
     *      Entry.
     *
     * @return
     *      Value for given entry.
     *      If no value is available empty string is returned.
     */
    std::string getValue(const Entry& entry) const;

    /**
     * Sets value for given entry.
     *
     * @param entry
     *      Entry.
     * @param value
     *      Value to set.
     */
    void setValue(const Entry& entry,
                  const std::string& value);

    /** The separate option information. */
    struct SeparateOptional
    {
        /** Flag indicating it was defined. */
        bool m_defined;
        /** Separate option key. */
        Key m_key;

        /**
         * Constructor.
         */
        SeparateOptional() :
                m_defined(false),
                m_key()
        {
            // noop
        }
    };

    /** Flag indicating if options are valid. */
    bool m_optionsValid;

    /** The separate option specified. */
    SeparateOptional m_separate;

    /** Available options. */
    std::vector<Entry> m_entries;

    /** Values set for the options. */
    std::map<Key, std::string> m_values;
};

} // namespace app
} // namespace subttxrend

#endif /*SUBTTXREND_APP_OPTIONS_HPP_*/
