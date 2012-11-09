#include "Precompile.h"

#include "Core\MSVCDependencyParser.h"
#include "Core\DependencyGraph.h"
#include "Core\DependencyView.h"
#include "Core\DependencyDiff.h"
#include "Utils\FileDictionary.h"

// ----------------------------------------------------------------------------
enum Columns
{
  e_FileName,
  e_FileCount,
  e_TotalScorePercent,
  e_TotalScoreValue,
  e_SelfScoreValue,
  e_ColumnCount,
};

class DependencyGraphModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  DependencyGraphModel(const GroupedDependencyGraph& graph, const FileDictionary& dict) 
    : m_Graph(graph)
    , m_Dict(dict)
  {
  }

  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const
  {
    return createIndex(row, column);
  }
  QModelIndex parent(const QModelIndex &child) const
  {
    return QModelIndex();
  }

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
  {
    if (role != Qt::DisplayRole)
      return QVariant();

    if (orientation == Qt::Horizontal)
    {
      switch(section)
      {
      case e_FileName: return QString("File name");
      case e_FileCount: return QString("File include");
      case e_TotalScorePercent: return QString("Total score(%)");
      case e_TotalScoreValue: return QString("Total score");
      case e_SelfScoreValue: return QString("Self score");
      default: return QString();
      }
    }
    else
      return QString("%1").arg(section + 1);
  }

  int rowCount(const QModelIndex &parent = QModelIndex()) const
  {
    const GroupedDependencyGraph::LinkArray& data = GetData();
    return data.GetCount();
  }
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const
  {
    return e_ColumnCount;
  }
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
  {
    if (!index.isValid())
      return QVariant();
    if (index.row() >= rowCount() )
      return QVariant();

    if (role != Qt::DisplayRole)
      return QVariant();

    const GroupedDependencyGraph::LinkArray& data = GetData();
    switch(index.column())
    {
    case e_FileName:
      {
        std::string filename;
        const GroupedDependencyGraph::Node* node = data.GetNode(index.row());
        return QString( m_Dict.GetFileName( node->GetData().fileHandle, filename).c_str() );
      }
    case e_FileCount:
      {
        const GroupedDependencyGraph::Link& link = data.GetLink(index.row());
        return QString::number(link.count);
      }
    case e_TotalScorePercent:
      {
        return QString("");
      }
    case e_TotalScoreValue:
      {
        const GroupedDependencyGraph::Node* node = data.GetNode(index.row());
        QString num;
        num.setNum(node->GetData().totalWeight, 'f', 0);
        return num;
      }
    case e_SelfScoreValue:
      {
        const GroupedDependencyGraph::Node* node = data.GetNode(index.row());
        QString num;
        num.setNum(node->GetData().myTotalWeight, 'f', 0);
        return num;
      }
    default:
      return QVariant();
    }

  }
  
  virtual const GroupedDependencyGraph::LinkArray& GetData() const = 0;
  virtual float GetTotalScore() const = 0;

protected:
  const GroupedDependencyGraph& m_Graph;
  const FileDictionary& m_Dict;
};

// ----------------------------------------------------------------------------
class ComplexityModel : public DependencyGraphModel
{
public:
  ComplexityModel(const GroupedDependencyGraph& graph, const FileDictionary& dict)
    : DependencyGraphModel(graph, dict)
  {

  }
  unsigned int FindRow(unsigned int fileHandle)
  {
    FindNodeByFileHandle match(fileHandle);
    unsigned int rowId;
    if ( GetData().FindNode(&match, rowId))
    {
      return rowId;
    }
    //assert
    return 0u;
  }
protected:
  const GroupedDependencyGraph::LinkArray& GetData() const
  {
    return m_Graph.GetHead()->GetChildren();
  }
  float GetTotalScore() const
  {
    return m_Graph.GetHead()->GetData().totalWeight;
  }

private:
  class FindNodeByFileHandle : public GroupedDependencyGraph::MatchNode
  {
  public:
    FindNodeByFileHandle(unsigned int fileHandle) : m_FileHandle(fileHandle) {}
    bool Match(const GroupedDependencyGraph::Node* node)
    {
      return node->GetData().fileHandle == m_FileHandle; 
    }
  private:
    unsigned int m_FileHandle;
  };

};

class DependantModel : public DependencyGraphModel
{
public:
  DependantModel(const GroupedDependencyGraph& graph, const FileDictionary& dict)
    : DependencyGraphModel(graph, dict)
    , m_ParentIndex(0u)
  {
  }
  void SetParentNode(unsigned int nodeIndex)
  {
    m_ParentIndex = nodeIndex;
    reset();
  }

  const GroupedDependencyGraph::Node* GetNode(unsigned int row)
  {
    return GetData().GetNode(row);
  }
protected:
  float GetTotalScore() const
  {
    return m_Graph.GetHead()->GetChildren().GetNode(m_ParentIndex)->GetData().totalWeight;
  }

  unsigned int m_ParentIndex;
};

class IncludeModel : public DependantModel
{
public:
  IncludeModel(const GroupedDependencyGraph& graph, const FileDictionary& dict)
    : DependantModel(graph, dict)
  {

  }
protected:
  const GroupedDependencyGraph::LinkArray& GetData() const
  {
    return m_Graph.GetHead()->GetChildren().GetNode(m_ParentIndex)->GetChildren();
  }
};

class RefByModel : public DependantModel
{
public:
  RefByModel(const GroupedDependencyGraph& graph, const FileDictionary& dict)
    : DependantModel(graph, dict)
  {

  }
  int columnCount(const QModelIndex &parent = QModelIndex()) const
  {
    return e_FileCount + 1;
  }
protected:
  const GroupedDependencyGraph::LinkArray& GetData() const
  {
    return m_Graph.GetHead()->GetChildren().GetNode(m_ParentIndex)->GetRefBy();
  }
};

// ----------------------------------------------------------------------------

class DependencyViewDelegate : public QItemDelegate
{
  Q_OBJECT
public:
  inline DependencyViewDelegate(DependencyGraphModel* obj) : QItemDelegate(obj) {}

  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
  {
    switch (index.column())
    {
    case e_TotalScorePercent:
      {
        // Set up a QStyleOptionProgressBar to precisely mimic the
        // environment of a progress bar.
        QStyleOptionProgressBarV2 progressBarOption;
        progressBarOption.state = QStyle::State_Enabled;
        progressBarOption.direction = QApplication::layoutDirection();
        progressBarOption.rect = option.rect;
        progressBarOption.fontMetrics = QApplication::fontMetrics();
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.textAlignment = Qt::AlignCenter;
        progressBarOption.textVisible = true;

        // Set the progress and text values of the style option.
        DependencyGraphModel* model = qobject_cast<DependencyGraphModel *>(parent());
        const GroupedDependencyGraph::LinkArray& data = model->GetData();
        float progress = 100.0f * data.GetNode(index.row())->GetData().totalWeight / model->GetTotalScore();
        progressBarOption.progress = progress;
        progressBarOption.text = QString().sprintf("%.2f%%", progress);

        // Draw the progress bar onto the view.
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
      }
      break;
    default:
      QItemDelegate::paint(painter, option, index);
    }

  }
};

// ----------------------------------------------------------------------------

class MainWindow : public QMainWindow
{
  Q_OBJECT
public: 
  MainWindow();
  ~MainWindow();

private slots:
  void BrowsePath();
  void SlotDescribeComplexityRow(const QItemSelection& selected, const QItemSelection& deselected);
  void SlotIncludeTableDoubleClick(const QModelIndex& index);
  void SlotRefByTableDoubleClick(const QModelIndex& index);
private:
  void SelectFileFromComplexityView(unsigned int fileHandle);
  void FillInformation(const char* filename);
  QWidget* m_MainForm;
  QPushButton* m_BrowsePathButton;
  QLineEdit*   m_Path;
  QTableView*  m_ComplexityTable;
  QTableView*  m_IncludeTable;
  QTableView*  m_RefByTable;

  DependencyGraph m_DependGraph;
  GroupedDependencyGraph  m_DependGroupedGraph;
  FileDictionary  m_FileDict;
  ComplexityModel* m_ComplexityGraphModel;
  IncludeModel* m_IncludeGraphModel;
  RefByModel* m_RefByGraphModel;
};

#include "moc/Main.moc"

MainWindow::MainWindow()
  :QMainWindow()
  , m_FileDict(FileDictionary::e_Compare_NoCase, 512)
  , m_DependGroupedGraph(512)
{
  QUiLoader loader;
  QFile file("forms/main_window.ui");
  file.open(QFile::ReadOnly);
  m_MainForm = loader.load(&file, NULL);
  file.close();
  m_MainForm->show();
  m_BrowsePathButton = m_MainForm->findChild<QPushButton*>("BrowsePathButton");
  m_Path = m_MainForm->findChild<QLineEdit*>("PathText");
  connect(m_BrowsePathButton, SIGNAL( clicked() ), this,  SLOT( BrowsePath() ));
  m_ComplexityTable = m_MainForm->findChild<QTableView*>("ComplexityTable");
  m_IncludeTable = m_MainForm->findChild<QTableView*>("IncludeTable");
  m_RefByTable = m_MainForm->findChild<QTableView*>("RefByTable");
  m_ComplexityTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  m_IncludeTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  m_RefByTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
}

MainWindow::~MainWindow()
{

}

void MainWindow::BrowsePath()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("*.*"));
  m_Path->setText( fileName );
  FillInformation(fileName.toAscii().data());
}

void MainWindow::FillInformation(const char* filename)
{
  MSVCDependencyParser parser;
  if (parser.ParseDenendencies(filename, m_DependGraph, m_FileDict) != MSVCDependencyParser::e_OK)
  {
    // output error
    return;
  }

  DependencyView::Build(m_DependGraph, m_FileDict, DependencyView::e_CompiledFilesCount, m_DependGroupedGraph);
  DependencyView::Sort(m_DependGroupedGraph);

  m_ComplexityGraphModel = new ComplexityModel(m_DependGroupedGraph, m_FileDict);
  m_IncludeGraphModel = new IncludeModel(m_DependGroupedGraph, m_FileDict);
  m_RefByGraphModel = new RefByModel(m_DependGroupedGraph, m_FileDict);

  m_ComplexityTable->setModel( m_ComplexityGraphModel );
  m_IncludeTable->setModel( m_IncludeGraphModel );
  m_RefByTable->setModel( m_RefByGraphModel );

  m_ComplexityTable->setItemDelegate(new DependencyViewDelegate(m_ComplexityGraphModel));
  m_IncludeTable->setItemDelegate(new DependencyViewDelegate(m_IncludeGraphModel));
  m_RefByTable->setItemDelegate(new DependencyViewDelegate(m_RefByGraphModel));

  connect(m_ComplexityTable->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(SlotDescribeComplexityRow(const QItemSelection&, const QItemSelection&)) );
  connect(m_IncludeTable, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(SlotIncludeTableDoubleClick(const QModelIndex&)));
  connect(m_RefByTable, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(SlotRefByTableDoubleClick(const QModelIndex&)));
}

void MainWindow::SlotDescribeComplexityRow(const QItemSelection& selected, const QItemSelection& deselected)
{
  QList<QModelIndex> selection = selected.indexes();
  // assert( selection.size() == 1);
  int rowIndex = selection.begin()->row();
  m_IncludeGraphModel->SetParentNode( rowIndex );
  m_RefByGraphModel->SetParentNode( rowIndex );
}

void MainWindow::SlotIncludeTableDoubleClick(const QModelIndex& index)
{
  SelectFileFromComplexityView(m_IncludeGraphModel->GetNode(index.row())->GetData().fileHandle);
}

void MainWindow::SlotRefByTableDoubleClick(const QModelIndex& index)
{
  SelectFileFromComplexityView(m_RefByGraphModel->GetNode(index.row())->GetData().fileHandle);
}

void MainWindow::SelectFileFromComplexityView( unsigned int fileHandle )
{
  unsigned int rowId = m_ComplexityGraphModel->FindRow(fileHandle);
  m_ComplexityTable->selectRow(rowId);
}

int main(int argc, char *argv[])  
{  
  QApplication app(argc, argv);
  MainWindow window;
  return app.exec();  
} 