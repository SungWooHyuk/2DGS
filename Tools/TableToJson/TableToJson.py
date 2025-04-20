import pyodbc
import json

def connect_to_db():
    conn = pyodbc.connect(
        'DRIVER={ODBC Driver 17 for SQL Server};'  
        'SERVER=localhost;'                        
        'DATABASE=2022-FALL;'                      
        'Trusted_Connection=yes;'                  
    )
    return conn

def fetch_item_data(conn):
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM item")

    columns = [column[0] for column in cursor.description]  

    items = []  

    for row in cursor.fetchall():  
        item = dict(zip(columns, row))  

        if item.get('item_type') == 1:
            equipment_cursor = conn.cursor()
            equipment_cursor.execute("SELECT * FROM item_equipment WHERE item_id = ?", item['item_id'])

            equipment_row = equipment_cursor.fetchone()
            if equipment_row:
                equip_columns = [column[0] for column in equipment_cursor.description]
                equipment_data = dict(zip(equip_columns, equipment_row))
                item['equipment_data'] = equipment_data  

        items.append(item)

    return items

def save_items_to_json(items, filename='items.json'):
    with open(filename, 'w', encoding='utf-8') as f:
        json.dump(items, f, indent=4, ensure_ascii=False)

def main():
    conn = connect_to_db()              
    items = fetch_item_data(conn)       
    save_items_to_json(items)           
    conn.close()                        

if __name__ == '__main__':
    main()