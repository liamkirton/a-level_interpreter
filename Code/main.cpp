//////////////////////////////////
// main.cpp
//////////////////////////////////
// Interpreter - ©Liam Kirton 2001
//////////////////////////////////

#include <windows.h>
#include <fstream.h>

#include "main.h"
#include "interpreter.h"

#include "about.h"

BOOL CALLBACK DlgProc (HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);

void SetupDialog();

void ButtonCheck(int x, int y, int t_check);
void ButtonClick(HWND hDlg, char *cmd);

void Paint(HWND hDlg, HDC hDC);

char *w_title;
char *w_about_txt;

output *t_output[50];
int t_output_count = 0;
code_output *c_output[50];
int c_output_count = 0;
button *t_buttons[50];
int t_button_count = 0;
text_label *labels[50];
int label_count = 0;

const int dWidth = 800;
const int dHeight = 600;

interpreter *p_int;

CRITICAL_SECTION cSection;
int exec_type = -3;

HANDLE hStartStop;
HANDLE hNext;
HANDLE hSignalLoadExit;
HANDLE hCloseApp;

output *p_output;
output *p_cfunc;
output *p_cline;
code_output *p_main;
code_output *p_current;

int Status = 1;
char *filename;

static bool bExit = false;

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	bool fix_exit = false;

	while (1)
	{
		WaitForSingleObject(hSignalLoadExit, INFINITE);
		
		EnterCriticalSection(&cSection);		
		if (Status == 0)
		{
			fix_exit = true;
			break;
		}
		LeaveCriticalSection(&cSection);

		p_int = new interpreter();
		
		EnterCriticalSection(&cSection);
		if (!(p_int->load(filename)))
		{
			exec_type = -2;
			delete p_int;
			LeaveCriticalSection(&cSection);
			continue;
		}
		LeaveCriticalSection(&cSection);
		
		try
		{
			if (!(p_int->run()))
			{
				exec_type = -2;			
			}
		}
		catch(...)
		{
			MessageBox(NULL, "Unhandled Error In Interpreter", "Error", MB_ICONEXCLAMATION);
			Status = 0;
			exec_type = -2;
			break;
		}
		delete p_int;
	}

	SetEvent(hCloseApp);
	if (fix_exit)
	{
		LeaveCriticalSection(&cSection);
	}
	ExitThread(0);
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nShowCmd)
{
	SetupDialog();

	w_title = new char[1024];
	strcpy(w_title, "Interpreter - Copyright © 2001 Liam Kirton 2001");

	w_about_txt = get_about_txt();

	filename = new char[1024];
	filename[0] = '\0';
	
	InitializeCriticalSection(&cSection);

	hStartStop = CreateEvent(0, false, false, "SIGNALEVENT");
	hNext = CreateEvent(0, false, false, "NEXTEVENT");
	hSignalLoadExit = CreateEvent(0, false, false, "LOADEXIT");
	hCloseApp = CreateEvent(0, false, false, "QUIT");

	HWND hDlg = CreateDialog(hInstance, "MAIN", NULL, DlgProc);
	SetWindowText(hDlg, w_title);

	MSG Msg;

	unsigned long *IntThreadID = new unsigned long;	
	HANDLE hIntThread = CreateThread(0, 0, ThreadProc, 0, CREATE_SUSPENDED, IntThreadID);
	SetThreadPriority(hIntThread, THREAD_PRIORITY_BELOW_NORMAL);
	if (hIntThread == NULL)
	{
		MessageBox(NULL, "Could Not Create Interpreter Thread!", "ERROR", MB_ICONEXCLAMATION);
		return -1;
	}

	ResumeThread(hIntThread);

	while (1)
	{		
		if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
		{
			if (Msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);				
		}
		else
		{
			EnterCriticalSection(&cSection);

			if (exec_type == -2)
			{
				exec_type = -3;
				strcpy(t_buttons[0]->txt, "OK");				
				t_buttons[0]->b_active = true;
				t_buttons[1]->b_active = false;
				t_buttons[2]->b_active = false;
				strcpy(t_buttons[2]->txt, "Step");
				t_buttons[3]->b_active = false;
				t_buttons[4]->b_active = true;
				t_buttons[5]->b_active = true;
				t_buttons[6]->b_active = true;

				t_buttons[0]->update = true;
				t_buttons[1]->update = true;
				t_buttons[2]->update = true;
				t_buttons[3]->update = true;
				t_buttons[4]->update = true;
				t_buttons[5]->update = true;
				t_buttons[6]->update = true;
								
				if (Status == 0)
				{
					SetEvent(hSignalLoadExit);
					LeaveCriticalSection(&cSection);
					WaitForSingleObject(hCloseApp, INFINITE);
					EnterCriticalSection(&cSection);
					PostMessage(hDlg, WM_CLOSE, 0, 0);
				}
			}
			else if (exec_type == -3)
			{
				if (Status == 0)
				{
					PostMessage(hDlg, WM_CLOSE, 0, 0);
				}
			}

			for (int i = 0; i < t_output_count; i++)
			{
				if (t_output[i]->update)
				{
					RECT dRect;
					dRect.left = t_output[i]->x;
					dRect.top = t_output[i]->y;
					dRect.right = t_output[i]->x + t_output[i]->width;
					dRect.bottom = t_output[i]->y + t_output[i]->height;
					
					InvalidateRect(hDlg, &dRect, false);

					t_output[i]->update = false;
				}
			}
			
			for (i = 0; i < c_output_count; i++)
			{
				if (c_output[i]->update)
				{
					RECT dRect;
					dRect.left = c_output[i]->x;
					dRect.top = c_output[i]->y;				
					dRect.right = c_output[i]->x + c_output[i]->width;
					dRect.bottom = c_output[i]->y + c_output[i]->height;
					
					InvalidateRect(hDlg, &dRect, false);

					c_output[i]->update = false;
				}
			}

			for (i = 0; i < t_button_count; i++)
			{
				if (t_buttons[i]->b_pressed)
				{
					ButtonClick(hDlg, t_buttons[i]->txt);
					t_buttons[i]->b_pressed = false;
				}
				
				if (t_buttons[i]->update)
				{
					RECT dRect;
					dRect.left = t_buttons[i]->x;
					dRect.top = t_buttons[i]->y;				
					dRect.right = t_buttons[i]->x + t_buttons[i]->width;
					dRect.bottom = t_buttons[i]->y + t_buttons[i]->height;
					
					InvalidateRect(hDlg, &dRect, false);

					t_buttons[i]->update = false;
				}
			}

			LeaveCriticalSection(&cSection);
			Sleep(0);
		}
	}

	delete [] filename;
	delete [] w_title;
	delete [] w_about_txt;

	for (int i = 0; i < t_output_count; i++)
	{
		delete t_output[i];
	}
	for (i = 0; i < t_button_count; i++)
	{
		delete t_buttons[i];
	}
	for (i = 0; i < c_output_count; i++)
	{
		delete c_output[i];
	}
	for (i = 0; i < label_count; i++)
	{
		delete labels[i];
	}

	return 0;
}

BOOL CALLBACK DlgProc (HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	static HDC hDC;
	static PAINTSTRUCT PaintSt;
	switch (Msg)
	{
		case WM_INITDIALOG:						
			SetWindowPos(hDlg, HWND_TOP, 0, 0, dWidth, dHeight, SWP_NOMOVE);
			ShowWindow (hDlg, SW_SHOW);	
			UpdateWindow (hDlg);
			InvalidateRect(hDlg, NULL, false);
			return true;

		case WM_LBUTTONDOWN:
			ButtonCheck(LOWORD(lParam), HIWORD(lParam), 1);
			return true;

		case WM_MOUSEMOVE:
			ButtonCheck(LOWORD(lParam), HIWORD(lParam), 0);
			return true;

		case WM_PAINT:
			hDC = BeginPaint(hDlg, &PaintSt);
			Paint(hDlg, hDC);
			EndPaint(hDlg, &PaintSt);
			return true;

		case WM_CLOSE:		
			DestroyWindow(hDlg);
			return true;

		case WM_DESTROY:
			PostQuitMessage(0);
			return true;

		default:
			return false;
	}
}

void SetupDialog()
{	
	/*--- OUTPUTS ---*/
	// Main[] Function Code
	c_output[0] = new code_output();
	c_output[0]->x = 150;
	c_output[0]->y = 20;
	c_output[0]->width = 600;
	c_output[0]->height = 100;
	c_output[0]->update = true;

	// Main Function Name Label
	labels[0] = new text_label();
	labels[0]->x = 5;
	labels[0]->y = 20;
	labels[0]->text = new char[100];
	strcpy(labels[0]->text, "Main Function Code:-");
	labels[0]->colour = RGB(250, 250, 250);
	labels[0]->update = true;

	// Current Function Name Display
	t_output[0] = new output();
	t_output[0]->x = 150;
	t_output[0]->y = 120;
	t_output[0]->width = 600;
	t_output[0]->height = 25;
	
	t_output[0]->l_display = 1;
	t_output[0]->colour = RGB(255,255,255);
	t_output[0]->update = true;

	// Current Function Name Label
	labels[1] = new text_label();
	labels[1]->x = 5;
	labels[1]->y = 120;
	labels[1]->text = new char[100];
	strcpy(labels[1]->text, "Current Function Name:-");
	labels[1]->colour = RGB(250, 250, 250);
	labels[1]->update = true;

	// Current Function Code
	c_output[1] = new code_output();
	c_output[1]->x = 150;
	c_output[1]->y = 145;
	c_output[1]->width = 600;
	c_output[1]->height = 125;
	c_output[1]->update = true;

	// Current Function Code
	labels[2] = new text_label();
	labels[2]->x = 5;
	labels[2]->y = 145;
	labels[2]->text = new char[100];
	strcpy(labels[2]->text, "Current Function Code:-");
	labels[2]->colour = RGB(250, 250, 250);
	labels[2]->update = true;

	// Current Line Processing
	t_output[1] = new output();
	t_output[1]->x = 150;
	t_output[1]->y = 270;
	t_output[1]->width = 600;
	t_output[1]->height = 125;

	t_output[1]->l_display = t_output[1]->height / 15 - 2;
	t_output[1]->colour = RGB(200, 200, 200);
	t_output[1]->update = true;

	// Current Line Processing Label
	labels[3] = new text_label();
	labels[3]->x = 5;
	labels[3]->y = 270;
	labels[3]->text = new char[100];
	strcpy(labels[3]->text, "Current Line Processing:-");
	labels[3]->colour = RGB(250, 250, 250);
	labels[3]->update = true;

	// Program Output
	t_output[2] = new output();
	t_output[2]->x = 150;
	t_output[2]->y = 395;
	t_output[2]->width = 600;
	t_output[2]->height = 125;
	t_output[2]->l_display = t_output[2]->height / 15 - 2;
	t_output[2]->colour = RGB(200, 200, 200);
	t_output[2]->update = true;

	// Program Output Label
	labels[4] = new text_label();
	labels[4]->x = 5;
	labels[4]->y = 395;
	labels[4]->text = new char[100];
	strcpy(labels[4]->text, "Program Output:-");
	labels[4]->colour = RGB(250, 250, 250);
	labels[4]->update = true;
	
	// Pointers Etc.
	c_output_count = 2;
	t_output_count = 3;
	
	p_main = c_output[0];
	p_current = c_output[1];
	p_cfunc = t_output[0];
	p_cline = t_output[1];
	p_output = t_output[2];

	/*--- BUTTONS ---*/
	
	// Buttons Label
	labels[5] = new text_label();
	labels[5]->x = 5;
	labels[5]->y = 540;
	labels[5]->text = new char[100];
	strcpy(labels[5]->text, "Control Buttons:-");
	labels[5]->colour = RGB(250, 250, 250);
	labels[5]->update = true;

	// Execution Options Label
	labels[6] = new text_label();
	labels[6]->x = 150;
	labels[6]->y = 540;
	labels[6]->text = new char[100];
	strcpy(labels[6]->text, "Execution:");
	labels[6]->colour = RGB(200, 200, 200);
	labels[6]->update = true;

	// Start: Run
	t_buttons[0] = new button();
	t_buttons[0]->x = 220;
	t_buttons[0]->y = 540;
	t_buttons[0]->width = 20;
	t_buttons[0]->height = 20;
	t_buttons[0]->txt = new char[1024];
	t_buttons[0]->b_active = false;
	t_buttons[0]->colour = RGB(0,255,0);
	t_buttons[0]->h_colour = RGB(0,0,255);
	strcpy(t_buttons[0]->txt, "Go!");

	// Start: Slow
	t_buttons[1] = new button();
	t_buttons[1]->x = 250;
	t_buttons[1]->y = 540;
	t_buttons[1]->width = 25;
	t_buttons[1]->height = 20;
	t_buttons[1]->txt = new char[1024];
	t_buttons[1]->b_active = false;
	t_buttons[1]->colour = RGB(0,255,0);
	t_buttons[1]->h_colour = RGB(0,0,255);
	strcpy(t_buttons[1]->txt, "Slow");

	// Continue: Step
	t_buttons[2] = new button();
	t_buttons[2]->x = 287;
	t_buttons[2]->y = 540;
	t_buttons[2]->width = 25;
	t_buttons[2]->height = 20;
	t_buttons[2]->txt = new char[1024];
	t_buttons[2]->b_active = false;
	t_buttons[2]->colour = RGB(0,255,0);
	t_buttons[2]->h_colour = RGB(0,0,255);
	strcpy(t_buttons[2]->txt, "Step");

	// Continue: Stop
	t_buttons[3] = new button();
	t_buttons[3]->x = 324;
	t_buttons[3]->y = 540;
	t_buttons[3]->width = 25;
	t_buttons[3]->height = 20;
	t_buttons[3]->txt = new char[1024];
	t_buttons[3]->b_active = false;
	t_buttons[3]->colour = RGB(0,255,0);
	t_buttons[3]->h_colour = RGB(0,0,255);
	strcpy(t_buttons[3]->txt, "Stop");
	
	// Interpreter Options Label
	labels[7] = new text_label();
	labels[7]->x = 450;	
	labels[7]->y = 540;
	labels[7]->text = new char[100];
	strcpy(labels[7]->text, "Interpreter:");
	labels[7]->colour = RGB(200, 200, 200);
	labels[7]->update = true;

	// New
	t_buttons[4] = new button();
	t_buttons[4]->x = 530;
	t_buttons[4]->y = 540;
	t_buttons[4]->width = 25;
	t_buttons[4]->height = 20;
	t_buttons[4]->txt = new char[1024];
	t_buttons[4]->b_active = true;
	t_buttons[4]->colour = RGB(0,255,0);
	t_buttons[4]->h_colour = RGB(0,0,255);
	strcpy(t_buttons[4]->txt, "New");

	// Load
	t_buttons[5] = new button();
	t_buttons[5]->x = 570;
	t_buttons[5]->y = 540;
	t_buttons[5]->width = 25;
	t_buttons[5]->height = 20;
	t_buttons[5]->txt = new char[1024];
	t_buttons[5]->b_active = true;
	t_buttons[5]->colour = RGB(0,255,0);
	t_buttons[5]->h_colour = RGB(0,0,255);
	strcpy(t_buttons[5]->txt, "Load");

	// Edit
	t_buttons[6] = new button();
	t_buttons[6]->x = 615;
	t_buttons[6]->y = 540;
	t_buttons[6]->width = 25;
	t_buttons[6]->height = 20;
	t_buttons[6]->txt = new char[1024];
	t_buttons[6]->b_active = false;
	t_buttons[6]->colour = RGB(0,255,0);
	t_buttons[6]->h_colour = RGB(0,0,255);
	strcpy(t_buttons[6]->txt, "Edit");

	// Exit
	t_buttons[7] = new button();
	t_buttons[7]->x = 655;
	t_buttons[7]->y = 540;
	t_buttons[7]->width = 25;
	t_buttons[7]->height = 20;
	t_buttons[7]->txt = new char[1024];
	t_buttons[7]->b_active = true;
	t_buttons[7]->colour = RGB(0,255,0);
	t_buttons[7]->h_colour = RGB(0,0,255);
	strcpy(t_buttons[7]->txt, "Exit");

	// About
	t_buttons[8] = new button();
	t_buttons[8]->x = 695;
	t_buttons[8]->y = 540;
	t_buttons[8]->width = 35;
	t_buttons[8]->height = 20;
	t_buttons[8]->txt = new char[1024];
	t_buttons[8]->b_active = true;
	t_buttons[8]->colour = RGB(0,255,0);
	t_buttons[8]->h_colour = RGB(0,0,255);
	strcpy(t_buttons[8]->txt, "About");

	t_button_count = 9;
	label_count = 8;

}

void ButtonCheck(int x, int y, int t_check)
{
	for (int i = 0; i < t_button_count; i++)
	{
		if (!t_buttons[i]->b_active)
		{
			continue;
		}

		if ((x >= t_buttons[i]->x) && (x <= (t_buttons[i]->x + t_buttons[i]->width))
			&& (y >= t_buttons[i]->y) && (y <= (t_buttons[i]->y + t_buttons[i]->height)))
		{
			if (t_check == 0)
			{
				if (!t_buttons[i]->b_highlighted)
				{
					t_buttons[i]->b_highlighted = true;
					t_buttons[i]->update = true;
				}
			}
			else
			{
				t_buttons[i]->b_pressed = true;
				t_buttons[i]->b_highlighted = false;
				t_buttons[i]->update = true;
			}
		}
		else
		{
			if (t_check == 0)
			{
				if (t_buttons[i]->b_highlighted)
				{
					t_buttons[i]->b_highlighted = false;
					t_buttons[i]->update = true;
					
				}
			}
		}
	}
}

void ButtonClick(HWND hDlg, char *txt)
{
	if ((strcmp(txt, "Go!")) == 0)
	{
		if (exec_type == 1)
		{
			exec_type = 0;
		}
		else if (exec_type == 2)
		{
			SetEvent(hNext);
			exec_type = 0;
		}
		else
		{
			SetEvent(hStartStop);
		}
		t_buttons[0]->b_active = false;
		t_buttons[0]->update = true;
		t_buttons[1]->b_active = false;
		t_buttons[1]->update = true;
		t_buttons[2]->b_active = false;
		t_buttons[2]->update = true;
		t_buttons[3]->b_active = false;
		t_buttons[3]->update = true;
	}
	else if ((strcmp(txt, "OK")) == 0)
	{
		t_buttons[0]->b_active = true;
		t_buttons[1]->b_active = true;
		t_buttons[2]->b_active = true;
		strcpy(t_buttons[2]->txt, "Step");
		t_buttons[3]->b_active = false;
		t_buttons[4]->b_active = false;
		t_buttons[5]->b_active = false;
		t_buttons[6]->b_active = false;

		t_buttons[0]->update = true;
		t_buttons[1]->update = true;
		t_buttons[2]->update = true;
		t_buttons[3]->update = true;
		t_buttons[4]->update = true;
		t_buttons[5]->update = true;
		t_buttons[6]->update = true;

		strcpy(t_buttons[0]->txt, "Go!");
		SetEvent(hSignalLoadExit);
	}
	else if ((strcmp(txt, "Slow")) == 0)
	{		
		if (exec_type != -3)
		{
			SetEvent(hNext);
		}
		else
		{
			SetEvent(hStartStop);
		}
		exec_type = 1;

		t_buttons[1]->b_active = false;
		t_buttons[1]->update = true;
		t_buttons[2]->b_active = false;
		t_buttons[2]->update = true;
		t_buttons[3]->b_active = true;
		t_buttons[3]->update = true;
	}
	else if ((strcmp(txt, "Step")) == 0)
	{
		strcpy(t_buttons[2]->txt, "Next");
		t_buttons[2]->update = true;
		t_buttons[3]->b_active = true;
		t_buttons[3]->update = true;
		exec_type = 2;
		SetEvent(hStartStop);
	}
	else if ((strcmp(txt, "Stop")) == 0)
	{
		t_buttons[3]->b_active = false;
		exec_type = -1;
		SetEvent(hNext);		
	}
	else if ((strcmp(txt, "Next")) == 0)
	{
		SetEvent(hNext);
	}
	else if ((strcmp(txt, "New")) == 0)
	{
		OPENFILENAME ofn;				

		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hDlg;
		ofn.lpstrFile = filename;
		ofn.nMaxFile = 1023;
		ofn.lpstrFilter = "Source Code Files (*.lan)\0*.lan\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
		ofn.lpstrTitle = "New";
		ofn.lpstrDefExt = ".lan";

		if ((GetSaveFileName(&ofn)) != 0)
		{
			ofstream *new_out = new ofstream(filename, ios::out);
			new_out->write("func null main[]\n{\n\n}\n{\n\n}", 26);
			new_out->close();
			delete new_out;

			char *cmdline = new char[1024];
			GetWindowsDirectory(cmdline, 1023);
			strcat(cmdline, "\\NOTEPAD.EXE ");
			strcat(cmdline, filename);
			WinExec(cmdline, SW_SHOWDEFAULT);
			delete [] cmdline;

			t_buttons[0]->b_active = true;
			strcpy(t_buttons[0]->txt, "OK");
			t_buttons[1]->b_active = false;
			t_buttons[2]->b_active = false;
			t_buttons[3]->b_active = false;
			t_buttons[4]->b_active = true;
			t_buttons[5]->b_active = true;
			t_buttons[6]->b_active = true;				
			t_buttons[0]->update = true;
			t_buttons[1]->update = true;
			t_buttons[2]->update = true;
			t_buttons[3]->update = true;
			t_buttons[4]->update = true;
			t_buttons[5]->update = true;
			t_buttons[6]->update = true;
		}
	}
	else if ((strcmp(txt, "Load")) == 0)
	{
		OPENFILENAME ofn;				

		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hDlg;
		ofn.lpstrFile = filename;
		ofn.nMaxFile = 1023;
		ofn.lpstrFilter = "Source Code Files (*.lan)\0*.LAN\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if ((GetOpenFileName(&ofn)) != 0)
		{						
			t_buttons[0]->b_active = true;
			strcpy(t_buttons[0]->txt, "OK");
			t_buttons[1]->b_active = false;
			t_buttons[2]->b_active = false;
			t_buttons[3]->b_active = false;
			t_buttons[4]->b_active = true;
			t_buttons[5]->b_active = true;
			t_buttons[6]->b_active = true;				
			t_buttons[0]->update = true;
			t_buttons[1]->update = true;
			t_buttons[2]->update = true;
			t_buttons[3]->update = true;
			t_buttons[4]->update = true;
			t_buttons[5]->update = true;
			t_buttons[6]->update = true;
		}		
	}
	else if ((strcmp(txt, "Edit")) == 0)
	{
		char *cmdline = new char[1024];
		GetWindowsDirectory(cmdline, 1023);
		strcat(cmdline, "\\NOTEPAD.EXE ");
		strcat(cmdline, filename);
		WinExec(cmdline, SW_SHOWDEFAULT);
		delete [] cmdline;
	}
	else if ((strcmp(txt, "Exit")) == 0)
	{
		if (exec_type != -3)
		{
			exec_type = -1;
			SetEvent(hNext);
		}
		Status = 0;
	}
	else if ((strcmp(txt, "About")) == 0)
	{
		MessageBox(hDlg, w_about_txt, "About Interpreter", MB_ICONINFORMATION);
		
	}
}

void PaintOutput(HWND hDlg, HDC hDC);

void Paint(HWND hDlg, HDC hDC)
{
	RECT BackRect;
	SelectObject(hDC, (HBRUSH)GetStockObject(BLACK_BRUSH));
	GetClientRect(hDlg, &BackRect);
	Rectangle(hDC, BackRect.left, BackRect.top, BackRect.right, BackRect.bottom);

	PaintOutput(hDlg, hDC);
}

void PaintOutput(HWND hDlg, HDC hDC)
{
	
	EnterCriticalSection(&cSection);

	static HFONT hFont = CreateFont(-12, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,						    
									OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
									PROOF_QUALITY, FIXED_PITCH, "Tahoma");
	
	static HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255,255,255));
	SelectObject(hDC, hPen);

	SelectObject(hDC, hFont);
	SetBkColor(hDC, (RGB(0,0,0)));

	for (int i = 0; i < label_count; i++)
	{
		RECT tRect;
		tRect.left = labels[i]->x;
		tRect.top = labels[i]->y;
		tRect.right = labels[i]->x + 150;
		tRect.bottom = labels[i]->y + 150;
		
		SetTextColor(hDC, labels[i]->colour);
		DrawText(hDC, labels[i]->text, strlen(labels[i]->text), &tRect, DT_LEFT);			
	}

	for (i = 0; i < c_output_count; i++)
	{	
		RECT cRect;
		cRect.left = c_output[i]->x;
		cRect.top = c_output[i]->y;
		cRect.right = c_output[i]->x + c_output[i]->width;
		cRect.bottom = c_output[i]->y + c_output[i]->height;

		Rectangle(hDC, cRect.left, cRect.top, cRect.right, cRect.bottom);

		if (!c_output[i]->initialised)
		{
			continue;
		}

		cRect.left += 5;
		cRect.top += 5;
		cRect.right -= 5;
		cRect.bottom -= 5;
	
		for (int j = c_output[i]->start; j <= c_output[i]->end; j++)
		{
			SetTextColor(hDC, c_output[i]->l_colour[j]);
			DrawText(hDC, c_output[i]->t_lines[j], strlen(c_output[i]->t_lines[j]), &cRect, DT_LEFT);
			cRect.top += 15;
		}
	}

	for (i = 0; i < t_output_count; i++)
	{
		RECT tRect;
		tRect.left = t_output[i]->x;
		tRect.top = t_output[i]->y;
		tRect.right = t_output[i]->x + t_output[i]->width;
		tRect.bottom = t_output[i]->y + t_output[i]->height;

		Rectangle(hDC, tRect.left, tRect.top, tRect.right, tRect.bottom);
		tRect.left += 5;
		tRect.top += 5;
		tRect.right -= 5;
		tRect.bottom -= 5;
	
		SetTextColor(hDC, t_output[i]->colour);

		for (int j = t_output[i]->start; j < t_output[i]->end; j++)
		{
			DrawText(hDC, t_output[i]->t_lines[j], strlen(t_output[i]->t_lines[j]), &tRect, DT_LEFT);
			tRect.top += 15;
		}
	}

	for (i = 0; i < t_button_count; i++)
	{
		RECT tRect;
		tRect.left = t_buttons[i]->x;
		tRect.top = t_buttons[i]->y;
		tRect.right = t_buttons[i]->x + t_buttons[i]->width;
		tRect.bottom = t_buttons[i]->y + t_buttons[i]->height;

		
		if (t_buttons[i]->b_active)
		{
			if (t_buttons[i]->b_highlighted)
			{
				SetTextColor(hDC, t_buttons[i]->h_colour);
			}
			else if (t_buttons[i]->b_pressed)
			{
				SetTextColor(hDC, t_buttons[i]->p_colour);
			}
			else
			{
				SetTextColor(hDC, t_buttons[i]->colour);
			}
		}
		else
		{
			SetTextColor(hDC, RGB(75,75,75));
		}

		DrawText(hDC, t_buttons[i]->txt, strlen(t_buttons[i]->txt), &tRect, DT_LEFT);
	}
	
	LeaveCriticalSection(&cSection);
}