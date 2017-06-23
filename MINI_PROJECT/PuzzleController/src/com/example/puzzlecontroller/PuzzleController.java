package com.example.puzzlecontroller;

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

public class PuzzleController extends Activity {
	
	public native void generate_puzzle(int row, int col);

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
	    System.loadLibrary("puzzlecontroller");
		
		Button buttonGen = (Button) findViewById(R.id.buttonGen);
        buttonGen.setOnClickListener( new OnClickListener() {
        	@SuppressWarnings("deprecation")
			@Override
        	public void onClick(View v) {
                EditText editTextRow = (EditText) findViewById(R.id.editTextRow);
                EditText editTextCol = (EditText) findViewById(R.id.editTextCol);
        		int row = Integer.parseInt(editTextRow.getText().toString());
        		int col = Integer.parseInt(editTextCol.getText().toString());
        		
        		generate_puzzle(row, col);
        	}
        });
	}
}
