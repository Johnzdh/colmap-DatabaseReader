#include <database.h>
#include <sqlite3_utils.h>
#include <string>
#include <mutex>

#include <iostream>

std::mutex Database::update_schema_mutex_;

Database::Database() : database_(nullptr) {}

Database::Database(const std::string& path) : Database() { Open(path); }

Database::~Database() { Close(); }

template <typename MatrixType>
MatrixType ReadDynamicMatrixBlob(sqlite3_stmt* sql_stmt, const int rc,
                                 const int col) {

  MatrixType matrix;

  if (rc == SQLITE_ROW) {
    const size_t rows =
        static_cast<size_t>(sqlite3_column_int64(sql_stmt, col + 0));
    const size_t cols =
        static_cast<size_t>(sqlite3_column_int64(sql_stmt, col + 1));

    matrix = MatrixType(rows, cols);

    const size_t num_bytes =
        static_cast<size_t>(sqlite3_column_bytes(sql_stmt, col + 2));

    memcpy(reinterpret_cast<char*>(matrix.data()),
           sqlite3_column_blob(sql_stmt, col + 2), num_bytes);
  } else {
    const typename MatrixType::Index rows =
        (MatrixType::RowsAtCompileTime == Eigen::Dynamic)
            ? 0
            : MatrixType::RowsAtCompileTime;
    const typename MatrixType::Index cols =
        (MatrixType::ColsAtCompileTime == Eigen::Dynamic)
            ? 0
            : MatrixType::ColsAtCompileTime;
    matrix = MatrixType(rows, cols);
  }

  return matrix;
}

cv::cvKps CVKeyPointsFromBlob(const FeatureKeypointsBlob& blob)
{
    cv::cvKps keypoints;

    if (blob.cols() == 2) 
    {
        for (FeatureKeypointsBlob::Index i = 0; i < blob.rows(); ++i) 
        {
            keypoints.emplace_back(blob(i, 0), blob(i, 1), 1);
        }
    }
    else
    {
        std::cout << "keypoints dimension is not correct! " << blob.cols() << std::endl;
    }
    
    return keypoints;
}

cv::cvDes CVDescriptorsFromBlob(const FeatureDescriptorsBlob colDescriptors)
{
    // TODO: Attention the data type!
    cv::cvDes cvDesriptors(colDescriptors.rows(), colDescriptors.cols(), CV_8UC1);
    cv::eigen2cv(colDescriptors, cvDesriptors);
    return cvDesriptors.clone();
}

void Database::FinalizeSQLStatements() {
  for (const auto& sql_stmt : sql_stmts_) {
    SQLITE3_CALL(sqlite3_finalize(sql_stmt));
  }
}

void Database::Close()
{
    if (database_ != nullptr) {
        FinalizeSQLStatements();
        sqlite3_close_v2(database_);
        database_ = nullptr;
    }
}

void Database::Open(const std::string& path)
{
    Close();

    SQLITE3_CALL(sqlite3_open_v2( path.c_str(), &database_,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, nullptr));
    
    // Don't wait for the operating system to write the changes to disk
    SQLITE3_EXEC(database_, "PRAGMA synchronous=OFF", nullptr);

    // Use faster journaling mode
    SQLITE3_EXEC(database_, "PRAGMA journal_mode=WAL", nullptr);

    // Store temporary tables and indices in memory
    SQLITE3_EXEC(database_, "PRAGMA temp_store=MEMORY", nullptr);

    // Disabled by default
    SQLITE3_EXEC(database_, "PRAGMA foreign_keys=ON", nullptr);


    PrepareSQLStatements();
}

void Database::PrepareSQLStatements() {
    sql_stmts_.clear();

    std::string sql;

    // num_*
    sql = "SELECT rows FROM keypoints WHERE image_id = ?;";
    SQLITE3_CALL(sqlite3_prepare_v2(database_, sql.c_str(), -1,
                                  &sql_stmt_num_keypoints_, 0));
    sql_stmts_.push_back(sql_stmt_num_keypoints_);

    sql = "SELECT rows FROM descriptors WHERE image_id = ?;";
    SQLITE3_CALL(sqlite3_prepare_v2(database_, sql.c_str(), -1,
                                  &sql_stmt_num_descriptors_, 0));
    sql_stmts_.push_back(sql_stmt_num_descriptors_);

    // exists_*

    sql = "SELECT 1 FROM cameras WHERE camera_id = ?;";
    SQLITE3_CALL(sqlite3_prepare_v2(database_, sql.c_str(), -1,
                                  &sql_stmt_exists_camera_, 0));
    sql_stmts_.push_back(sql_stmt_exists_camera_);

    sql = "SELECT 1 FROM images WHERE image_id = ?;";
    SQLITE3_CALL(sqlite3_prepare_v2(database_, sql.c_str(), -1,
                                  &sql_stmt_exists_image_id_, 0));
    sql_stmts_.push_back(sql_stmt_exists_image_id_);

    sql = "SELECT 1 FROM images WHERE name = ?;";
    SQLITE3_CALL(sqlite3_prepare_v2(database_, sql.c_str(), -1,
                                  &sql_stmt_exists_image_name_, 0));
    sql_stmts_.push_back(sql_stmt_exists_image_name_);

    sql = "SELECT 1 FROM keypoints WHERE image_id = ?;";
    SQLITE3_CALL(sqlite3_prepare_v2(database_, sql.c_str(), -1,
                                  &sql_stmt_exists_keypoints_, 0));
    sql_stmts_.push_back(sql_stmt_exists_keypoints_);

    sql = "SELECT 1 FROM descriptors WHERE image_id = ?;";
    SQLITE3_CALL(sqlite3_prepare_v2(database_, sql.c_str(), -1,
                                  &sql_stmt_exists_descriptors_, 0));
    sql_stmts_.push_back(sql_stmt_exists_descriptors_);


    // read images
    sql = "SELECT * FROM images;";
    SQLITE3_CALL(sqlite3_prepare_v2(database_, sql.c_str(), -1,
                                    &sql_stmt_read_images_, 0));
    sql_stmts_.push_back(sql_stmt_read_images_);

    sql = "SELECT rows, cols, data FROM keypoints WHERE image_id = ?;";
    SQLITE3_CALL(sqlite3_prepare_v2(database_, sql.c_str(), -1,
                                    &sql_stmt_read_keypoints_, 0));
    sql_stmts_.push_back(sql_stmt_read_keypoints_);

    sql = "SELECT rows, cols, data FROM descriptors WHERE image_id = ?;";
    SQLITE3_CALL(sqlite3_prepare_v2(database_, sql.c_str(), -1,
                                    &sql_stmt_read_descriptors_, 0));
    sql_stmts_.push_back(sql_stmt_read_descriptors_);

}

cv::cvKps Database::ReadKeyPoints(const image_t image_id) const {
  SQLITE3_CALL(sqlite3_bind_int64(sql_stmt_read_keypoints_, 1, image_id));

  const int rc = SQLITE3_CALL(sqlite3_step(sql_stmt_read_keypoints_));
  const FeatureKeypointsBlob blob = ReadDynamicMatrixBlob<FeatureKeypointsBlob>(sql_stmt_read_keypoints_, rc, 0);

  SQLITE3_CALL(sqlite3_reset(sql_stmt_read_keypoints_));

  return CVKeyPointsFromBlob(blob);
}

cv::cvDes Database::ReadDescriptors(const image_t image_id) const {
    SQLITE3_CALL(sqlite3_bind_int64(sql_stmt_read_descriptors_, 1, image_id));

    const int rc = SQLITE3_CALL(sqlite3_step(sql_stmt_read_descriptors_));
    const FeatureDescriptorsBlob descriptors = ReadDynamicMatrixBlob<FeatureDescriptorsBlob>(sql_stmt_read_descriptors_, rc, 0);

    SQLITE3_CALL(sqlite3_reset(sql_stmt_read_descriptors_));

    return CVDescriptorsFromBlob(descriptors);
}