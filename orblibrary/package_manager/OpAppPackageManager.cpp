#include "OpAppPackageManager.h"
#include <mutex>

// Static member initialization
std::unique_ptr<OpAppPackageManager> OpAppPackageManager::s_Instance = nullptr;
std::mutex OpAppPackageManager::s_InstanceMutex;

OpAppPackageManager::OpAppPackageManager(const Configuration& configuration)
  : m_IsRunning(false),
    m_IsUpdating(false),
    m_Configuration(configuration)
{
}

OpAppPackageManager::~OpAppPackageManager()
{
  // Ensure the worker thread is stopped before destruction
  if (m_IsRunning) {
    stop();
  }
}

// Singleton instance management
OpAppPackageManager* OpAppPackageManager::getInstance()
{
  std::lock_guard<std::mutex> lock(s_InstanceMutex);

  // Return nullptr if no instance has been created yet
  return s_Instance.get();
}

OpAppPackageManager& OpAppPackageManager::getInstance(const Configuration& configuration)
{
  std::lock_guard<std::mutex> lock(s_InstanceMutex);

  if (!s_Instance) {
    s_Instance = std::unique_ptr<OpAppPackageManager>(new OpAppPackageManager(configuration));
  }
  // If instance already exists, we just return the existing instance
  // This is a design decision - you could also log a warning or handle it differently

  return *s_Instance;
}

void OpAppPackageManager::destroyInstance()
{
  std::lock_guard<std::mutex> lock(s_InstanceMutex);
  if (s_Instance) {
    s_Instance.reset();
    s_Instance = nullptr; // Explicitly set to nullptr to ensure it's destroyed
  }
}

void OpAppPackageManager::start()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  if (m_IsRunning) {
    return;
  }

  m_WorkerThread = std::thread(&OpAppPackageManager::checkForUpdates, this);

  m_IsRunning = true;
  m_IsUpdating = false;
}

void OpAppPackageManager::stop()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  if (!m_IsRunning) {
    return;
  }

  m_WorkerThread.join();

  m_IsRunning = false;
  m_IsUpdating = false;
}

bool OpAppPackageManager::isRunning() const
{
  return m_IsRunning;
}

bool OpAppPackageManager::isUpdating() const
{
    return m_IsUpdating;
}

void OpAppPackageManager::checkForUpdates()
{
}
