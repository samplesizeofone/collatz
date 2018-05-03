#!/usr/bin/env wolframscript
UsingFrontEnd[
    {inFileName, outFileName} = Take[$ScriptCommandLine, {2, 3}];
    nb = NotebookOpen[inFileName];

    FrontEndExecute[
        FrontEndToken[
            nb,
            "Save",
            {outFileName, "Package"}
        ]
    ]
];
