#pragma once
#include<string>
#include<memory>
#include<vector>
#include<map>
#include"ResourceManagement/ISerializationInterface.h"

namespace Databases
{
	using Serialzer_t = typename ResourceManagement::Serialization::Serialzer_t;

	//data types stored in the data base
	enum class ECellDataType
	{
		BOOL,
		INT,
		FLOAT,
		STRING,
		INVALID
	};

	struct FDataBase : ResourceManagement::Serialization::ISerializationInterface
	{
		virtual ECellDataType GetType() const= 0;
	};
	
	struct FBoolData : public FDataBase
	{
		bool Data{};
		ECellDataType GetType() const { return ECellDataType::BOOL; }
		virtual bool Load(const Serialzer_t& Archive)
		{
			LoadProp(Archive, Data);
			return true;
		}
		virtual void Save(Serialzer_t& Archive)
		{
			SaveProp(Archive, Data);
		}
	};

	struct FIntData : public FDataBase
	{
		ECellDataType GetType() const { return ECellDataType::INT; }
		virtual bool Load(const Serialzer_t& Archive)
		{
			LoadProp(Archive, Data);
			return true;
		}
		virtual void Save(Serialzer_t& Archive)
		{
			SaveProp(Archive, Data);
		}
		int Data{};
	};

	struct FFloatData : public FDataBase
	{
		ECellDataType GetType() const { return ECellDataType::FLOAT; }
		virtual bool Load(const Serialzer_t& Archive)
		{
			LoadProp(Archive, Data);
			return true;
		}
		virtual void Save(Serialzer_t& Archive)
		{
			SaveProp(Archive, Data);
		}
		float Data{};
	};

	struct FStringData : public FDataBase
	{
		ECellDataType GetType() const { return ECellDataType::STRING; }
		virtual bool Load(const Serialzer_t& Archive)
		{
			LoadProp(Archive, Data);
			return true;
		}
		virtual void Save(Serialzer_t& Archive)
		{
			SaveProp(Archive, Data);
		}
		std::string Data{};
	};


	std::shared_ptr<FBoolData> CreateBool();
	std::shared_ptr<FIntData> CreateInt();
	std::shared_ptr<FFloatData> CreateFloat();
	std::shared_ptr<FStringData> CreateString();

	std::shared_ptr<FDataBase> CreateDataFromType(ECellDataType type);

	bool DoesDataMatchType(const std::shared_ptr<FDataBase>& dat, ECellDataType Type);

	class DataCell : ResourceManagement::Serialization::ISerializationInterface
	{
		std::shared_ptr<FDataBase> m_Data{};
		
	public:
		std::shared_ptr<FDataBase> Get() const { return m_Data; };
		ECellDataType GetType() const;

		bool IsValid()const;

		void Set(std::shared_ptr<FDataBase> val);
		
		void Clear();

		std::shared_ptr<FBoolData> GetAsBool()const;
		std::shared_ptr<FIntData> GetAsInt()const;
		std::shared_ptr<FFloatData> GetAsFloat()const;
		std::shared_ptr<FStringData> GetAsString()const;

		virtual bool Load(const Serialzer_t& Archive);
		virtual void Save(Serialzer_t& Archive);
		virtual void Unload();
	};

	class DataRow : public ResourceManagement::Serialization::ISerializationInterface
	{
	public:

		int GetSize()const { return m_Cells.size(); };
		std::vector<std::shared_ptr<DataCell>>& GetCells() { return m_Cells; }

		std::shared_ptr<DataCell> GetCell(int index)const;

		void AddCell(ECellDataType CellType);
		void InsertCell(ECellDataType CellType, int index);

		void RemoveCell(int index);

		bool IsValidIndex(int index)const;

		void Clear();

		virtual bool Load(const Serialzer_t& Archive);
		virtual void Save(Serialzer_t& Archive);
		virtual void Unload();
	private:
		std::vector<std::shared_ptr<DataCell>> m_Cells;
	};

	struct TableColumn
	{
		std::string m_Name;
		ECellDataType m_DataType;
	};

	class DataTable : public ResourceManagement::Serialization::ISerializationInterface
	{
		std::vector<std::shared_ptr<DataRow>> m_Rows;
		std::vector<TableColumn> m_Columns;
		std::string m_TableName;

		std::shared_ptr<DataRow> CreateNewRow();
	public:
		void AddColumn(std::string name, ECellDataType type);
		void RemoveColumn(int index);
		void InsertColumns(std::string name, ECellDataType type, int index);
		const TableColumn& GetColumn(int index)const;

		std::shared_ptr<DataRow> GetRow(int index)const;
		void AddRow();
		void InsertRow(int index);
		void RemoveRow(int index);

		int GetRowsCount()const;
		int GetColumnsCount()const;
		std::string_view GetTableName()const;

		bool IsValidRowIndex(int index)const;
		bool IsValidColumnIndex(int index)const;

		void Clear(bool clearColumnsToo = false);

		void SetTableName(std::string newname);
		void SetColumnName(int index, std::string newname);

		virtual bool Load(const Serialzer_t& Archive);
		virtual void Save(Serialzer_t& Archive);
		virtual void Unload();
	};
}
