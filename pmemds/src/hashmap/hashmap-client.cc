namespace pmemdscleint {
    class HashMapEngine : public PMEngine {
    public:
        HashMapEngine(const std::string &path, size_t size);          // default constructor
        ~HashMapEngine();                                        // default destructor

        std::string Engine() final { return ENGINE; }               // engine identifier

        PMStatus Exists(const std::string &key) final;              // does key have a value?

        void get(void *context,                                // pass value to callback with context
                 const std::string &key,
                 PMGetCallback *callback) final;

        PMStatus put(const std::string &key,                        // store key and value
                     const std::string &value) final;

        PMStatus remove(const std::string &key) final;              // remove value for key
    private:
        HashMapEngine(const BTreeEngine &);

        void operator=(const BTreeEngine &);


    };

}