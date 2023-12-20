#pragma once
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "szk_assert.h"
namespace util {
class Option {
 public:
    Option(const std::string& s,
           const std::string& l) :
        _short(s), _long(l), _set(false) {
        check();
    }
    virtual ~Option() {}

    virtual bool needArg() const = 0;
    virtual void process(const char*) = 0;

    virtual bool enabled() const;
    virtual long getLong() const;
    virtual unsigned long getUlong()  const;
    virtual int getInt() const;
    virtual unsigned int getUint() const;
    virtual const std::string& getString() const;
    virtual double getReal() const;

    bool isSet() const { return _set; }
    void set() { _set = true; }

    const std::string& shortName() const { return _short; }
    const std::string& LongName() const { return _long; }

 private:
    void check();

 private:
    const std::string           _short;
    const std::string           _long;
    bool                        _set;
};

class BoolOption : public Option {
 public:
    BoolOption(const std::string& s,
                const std::string& l,
                bool d) :
               Option(s, l), _enabled(d) {}

    bool needArg() const final { return false; }
    bool enabled() const final { return _enabled; }
    void process(const char*) final { _enabled = true; }

 private:
    bool                        _enabled;
};

class LongOption : public Option {
    LongOption(const std::string& s,
                const std::string& l,
                long d) :
               Option(s, l), _v(d) {}

    bool needArg() const final { return true; }
    long getLong() const final { return _v; }
    void process(const char*) final;

 private:
    long                        _v;
};

class UlongOption : public Option {
    UlongOption(const std::string& s,
                const std::string& l,
                unsigned long d) :
               Option(s, l), _v(d) {}

    bool needArg() const final { return true; }
    unsigned long getUlong() const final { return _v; }
    void process(const char*) final;

 private:
    unsigned long               _v;
};

class IntOption : public Option {
    IntOption(const std::string& s,
                const std::string& l,
                int d) :
               Option(s, l), _v(d) {}

    bool needArg() const final { return true; }
    int getInt() const final { return _v; }
    void process(const char*) final;

 private:
    int                         _v;
};

class UintOption : public Option {
    UintOption(const std::string& s,
                const std::string& l,
                unsigned int d) :
               Option(s, l), _v(d) {}

    bool needArg() const final { return true; }
    unsigned int getUint() const final { return _v; }
    void process(const char*) final;

 private:
    unsigned int                _v;
};

class StringOption : public Option {
    StringOption(const std::string& s,
                const std::string& l,
                const std::string& d) :
               Option(s, l), _v(d) {}

    bool needArg() const final { return true; }
    const std::string& getString() const final { return _v; }
    void process(const char*) final;

 private:
    std::string                 _v;
};

class RealOption : public Option {
    RealOption(const std::string& s,
                const std::string& l,
                double d) :
               Option(s, l), _v(d) {}

    bool needArg() const final { return true; }
    double getReal() const final { return _v; }
    void process(const char*) final;

 private:
    double                      _v;
};

class OptionDB {
 public:
    typedef std::vector<std::string> ArgVec;

    OptionDB() {}

    static OptionDB& get() { return gSingleton; }

    void addOption(size_t id, Option* opt);
    void parse(int argc, const char* argv[]);

    const ArgVec& getArgs() const { return _args; }

    static const Option& getOption(size_t id) {
        Assert(id < gSingleton._holder.size());
        return *gSingleton._holder[id];
    }

 private:
    typedef std::unordered_map<std::string, Option*> OptionIndex;
    typedef std::unique_ptr<Option> OptionPtr;
    typedef std::vector<OptionPtr> OptionHolder;

 private:
    OptionIndex                     _index;
    OptionHolder                    _holder;
    ArgVec                          _args;

    static OptionDB                 gSingleton;
};

}
