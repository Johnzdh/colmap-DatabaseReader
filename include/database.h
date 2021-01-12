#ifndef SRC_DATABASE_
#define SRC_DATABASE_

#include <string>
#include <sqlite3.h>

#include "type_utils.h"


class Database{
    public:
    Database();
    explicit Database(const std::string& path);
    ~Database();

    // Open and close the database
    void Open(const std::string& path);
    void Close();

    // Sum of `rows` column in `keypoints` table, i.e. number of total keypoints.
    size_t NumKeypoints() const;

    // The number of keypoints for the image with most features.
    size_t MaxNumKeypoints() const;

    // Sum of `rows` column in `descriptors` table,
    size_t NumDescriptors() const;

    // The number of descriptors for the image with most features.
    size_t MaxNumDescriptors() const;

    cv::cvDes ReadDescriptors(const image_t image_id) const;
    cv::cvKps ReadKeyPoints(const image_t image_id) const; 

    void PrepareSQLStatements();
    void FinalizeSQLStatements();

    private:
    
    sqlite3* database_ = nullptr;

    // A collection of all `sqlite3_stmt` objects for deletion in the destructor.
    std::vector<sqlite3_stmt*> sql_stmts_;

    // num_*
    sqlite3_stmt* sql_stmt_num_keypoints_ = nullptr;
    sqlite3_stmt* sql_stmt_num_descriptors_ = nullptr;

    // exists_*
    sqlite3_stmt* sql_stmt_exists_camera_ = nullptr;
    sqlite3_stmt* sql_stmt_exists_image_id_ = nullptr;
    sqlite3_stmt* sql_stmt_exists_image_name_ = nullptr;
    sqlite3_stmt* sql_stmt_exists_keypoints_ = nullptr;
    sqlite3_stmt* sql_stmt_exists_descriptors_ = nullptr;

    // read_*
    sqlite3_stmt* sql_stmt_read_images_ = nullptr;
    sqlite3_stmt* sql_stmt_read_keypoints_ = nullptr;
    sqlite3_stmt* sql_stmt_read_descriptors_ = nullptr;

    // Ensure that only one database object at a time updates the schema of a
    // database. Since the schema is updated every time a database is opened, this
    // is to ensure that there are no race conditions ("database locked" error
    // messages) when the user actually only intends to read from the database,
    // which requires to open it.
    static std::mutex update_schema_mutex_;

};

#endif //SRC_DATABASE_