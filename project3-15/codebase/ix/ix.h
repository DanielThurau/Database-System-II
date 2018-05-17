#ifndef _ix_h_
#define _ix_h_

#include <vector>
#include <string>
#include <cstring>

#include "../rbf/rbfm.h"

#define IX_EOF (-1)  // end of the index scan

#define SUCCESS 0

#define IX_CREATE_FAILED  1
#define IX_MALLOC_FAILED  2
#define IX_OPEN_FAILED    3
#define IX_APPEND_FAILED  4
#define IX_READ_FAILED    5
#define IX_WRITE_FAILED   6

typedef enum {
    INTERIOR_NODE = 0,
    LEAF_NODE
} NodeType;


typedef struct IndexDirectory {
    uint32_t numEntries;
    uint32_t freeSpaceOffset;
    NodeType type;
} IndexDirectory;

// Struct for leaf page sibling references
// -1 indicated null reference if furthest 
// left/right sibling
typedef struct FamilyDirectory {
    PageNum parent;
    int32_t leftSibling;
    int32_t rightSibling;
} FamilyDirectory;

class InteriorNode {
public:
    InteriorNode(const void *page, const Attribute &attribute);
    RC writeToPage(void *page, Attribute &attribute);

    IndexDirectory  indexDirectory;
    FamilyDirectory familyDirectory;

    vector<void *> trafficCops;
    vector<PageNum> pagePointers;

};

class LeafNode {
public:
    LeafNode(const void *page, const Attribute &attribute);
    RC writeToPage(void *page, Attribute &attribute);

    IndexDirectory  indexDirectory;
    FamilyDirectory familyDirectory;

    vector<void*> keys;
    vector<RID> rids;
};

class IX_ScanIterator;
class IXFileHandle;

class IndexManager {

    public:
        static IndexManager* instance();

        // Create an index file.
        RC createFile(const string &fileName);

        // Delete an index file.
        RC destroyFile(const string &fileName);

        // Open an index and return an ixfileHandle.
        RC openFile(const string &fileName, IXFileHandle &ixfileHandle);

        // Close an ixfileHandle for an index.
        RC closeFile(IXFileHandle &ixfileHandle);

        // Insert an entry into the given index that is indicated by the given ixfileHandle.
        RC insertEntry(IXFileHandle &ixfileHandle, const Attribute &attribute, const void *key, const RID &rid);

        // Delete an entry from the given index that is indicated by the given ixfileHandle.
        RC deleteEntry(IXFileHandle &ixfileHandle, const Attribute &attribute, const void *key, const RID &rid);

        // Initialize and IX_ScanIterator to support a range search
        RC scan(IXFileHandle &ixfileHandle,
                const Attribute &attribute,
                const void *lowKey,
                const void *highKey,
                bool lowKeyInclusive,
                bool highKeyInclusive,
                IX_ScanIterator &ix_ScanIterator);

        // Print the B+ tree in pre-order (in a JSON record format)
        void printBtree(IXFileHandle &ixfileHandle, const Attribute &attribute) const;

        friend class InteriorNode;
        friend class LeafNode;

    protected:
        IndexManager();
        ~IndexManager();

    private:
        static IndexManager *_index_manager;
        static PagedFileManager *_pf_manager;


        void newLeafBasedPage(void *page, int32_t leftSibling, int32_t rightSibling, PageNum parent);
        void newInteriorBasedPage(void *page, int32_t leftSibling, int32_t rightSibling, PageNum parent);

        void getIndexDirectory(const void *page, IndexDirectory &directory);
        void setIndexDirectory(void *page, IndexDirectory &directory);

        void getRootPage(IXFileHandle &ixfileHandle, void *page);
        NodeType getNodeType(const void *page);
        int compareAttributeValues(const void *key_1, const void *key_2, const Attribute &attribute);
        void findPageWithKey(IXFileHandle &ixfileHandle, const void *key, const  Attribute &attribute, void *page);
        void getFamilyDirectory(const void *page, FamilyDirectory &directory);
        void setFamilyDirectory(void *page, FamilyDirectory &directory);
};


class IX_ScanIterator {
    public:

		// Constructor
        IX_ScanIterator();

        // Destructor
        ~IX_ScanIterator();

        // Get next matching entry
        RC getNextEntry(RID &rid, void *key);

        // Terminate index scan
        RC close();
};



class IXFileHandle {
    public:

        // variables to keep counter for each operation
        unsigned ixReadPageCounter;
        unsigned ixWritePageCounter;
        unsigned ixAppendPageCounter;

        FileHandle* fileHandle;

        // Constructor
        IXFileHandle();

        // Destructor
        ~IXFileHandle();

    	// Put the current counter values of associated PF FileHandles into variables
    	RC collectCounterValues(unsigned &readPageCount, unsigned &writePageCount, unsigned &appendPageCount);
        RC readPage(PageNum pageNum, void *data);                           // Get a specific page
        RC writePage(PageNum pageNum, const void *data);                    // Write a specific page
        RC appendPage(const void *data);                                    // Append a specific page
        unsigned getNumberOfPages();

};

#endif
