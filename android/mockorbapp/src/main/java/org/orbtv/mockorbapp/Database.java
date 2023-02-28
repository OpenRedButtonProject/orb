/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

package org.orbtv.mockorbapp;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.provider.BaseColumns;

import java.util.Locale;

public class Database extends SQLiteOpenHelper {
   // Important: The database version must be incremented with each change to the schema, and
   // for each change to the schema, whether upgrade/downgrade is adequate should be considered
   public static final String DATABASE_NAME = "TvBrowserDatabase%d.db";
   public static final int DATABASE_VERSION = 4;

   public static class DistinctiveIdentifierEntry implements BaseColumns {
      public static final String TABLE_NAME = "key_value";
      public static final String COLUMN_NAME_VALUE = "value";
   }

   public Database(Context context, int profileId) {
      super(context, String.format(Locale.ENGLISH, DATABASE_NAME, profileId), null, DATABASE_VERSION);
   }

   public void onCreate(SQLiteDatabase db) {
      db.execSQL(SQL_CREATE_KEY_VALUE_TABLE);
   }

   public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
      // Presently the upgrade strategy is to start over
      db.execSQL(SQL_DELETE_KEY_VALUE_TABLE);
      onCreate(db);
   }

   public void onDowngrade(SQLiteDatabase db, int oldVersion, int newVersion) {
      onUpgrade(db, oldVersion, newVersion);
   }

   // Distinctive identifiers
   public boolean hasDistinctiveIdentifier(String origin) {
      SQLiteDatabase db = getReadableDatabase();
      String selection = Database.DistinctiveIdentifierEntry._ID + " = ?";
      String[] selectionArgs = { origin };
      Cursor cursor = db.query(Database.DistinctiveIdentifierEntry.TABLE_NAME, null, selection,
         selectionArgs, null, null, null,"1");
      boolean has = (cursor.getCount() > 0);
      cursor.close();
      return has;
   }

   public String getDistinctiveIdentifier(String origin) {
      SQLiteDatabase db = getReadableDatabase();
      String selection = Database.DistinctiveIdentifierEntry._ID + " = ?";
      String[] selectionArgs = { origin };
      Cursor cursor = db.query(Database.DistinctiveIdentifierEntry.TABLE_NAME, null, selection,
         selectionArgs, null, null, null,"1");
      String value = null;
      if (cursor.getCount() > 0) {
         cursor.moveToFirst();
         int column = cursor.getColumnIndex(DistinctiveIdentifierEntry.COLUMN_NAME_VALUE);
         if (column >= 0) {
            value = cursor.getString(column);
         }
      }
      cursor.close();
      return value;
   }

   public boolean setDistinctiveIdentifier(String origin, String distinctiveIdentifier) {
      SQLiteDatabase db = getWritableDatabase();
      if (distinctiveIdentifier == null) {
         String selection = Database.DistinctiveIdentifierEntry._ID + " = ?";
         String[] selectionArgs = { origin };
         db.delete(Database.DistinctiveIdentifierEntry.TABLE_NAME, selection, selectionArgs);
         return true;
      } else {
         ContentValues values = new ContentValues();
         values.put(Database.DistinctiveIdentifierEntry._ID, origin);
         values.put(Database.DistinctiveIdentifierEntry.COLUMN_NAME_VALUE, distinctiveIdentifier);
         long newRowId = db.replace(Database.DistinctiveIdentifierEntry.TABLE_NAME, null, values);
         return (newRowId != -1);
      }
   }

   public void deleteDistinctiveIdentifier(String origin) {
      setDistinctiveIdentifier(origin, null);
   }

   public void deleteAllDistinctiveIdentifiers() {
      SQLiteDatabase db = getWritableDatabase();
      String[] selectionArgs = {};
      db.delete(Database.DistinctiveIdentifierEntry.TABLE_NAME, null, selectionArgs);
   }

   private static final String SQL_CREATE_KEY_VALUE_TABLE =
      "CREATE TABLE " + DistinctiveIdentifierEntry.TABLE_NAME + " (" +
         DistinctiveIdentifierEntry._ID + " TEXT PRIMARY KEY," +
         DistinctiveIdentifierEntry.COLUMN_NAME_VALUE + " TEXT)";

   private static final String SQL_DELETE_KEY_VALUE_TABLE =
      "DROP TABLE IF EXISTS " + DistinctiveIdentifierEntry.TABLE_NAME;
}

