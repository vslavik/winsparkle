/* 
	Documentation Insight
	Copyright (C) 2010-2013 DevJet Software
*/

$(function () {
    // Hide inherited members in design mode by default
    showInheritedMembers = false;
    updateInheritedMembers();

    watchContentEditableElements();

    $('a').attr('tabindex', '-1');
    $('div[contentEditable="true"] a').removeAttr('tabindex');
    $('input').attr('tabindex', '-1');

    if (contentEditableControl.value == 'true') {
        AddRowToTable('exceptions', 'exception');
        AddRowToTable('permissions', 'permission');
        AddRowToTable('authors', 'author');
        AddRowToTable('seealso', 'seealso');
        AddRowToDefTables();
    }
});

function notifyModified() {
    var memberId;
    memberId = getActiveMemberId();
    if (memberId) {
        if (window.external && ('NotifyMemberModified' in window.external)) {
            window.external.notifyMemberModified(memberId);
        }        
        return;
    }
    if (window.external && ('MarkAsModified' in window.external)) {
        window.external.markAsModified();
    }
}

function getActiveMemberElement() {
    var element = $(document.activeElement).closest('div.memberSummary[contentEditable="true"]');
    return element;
}

function getActiveMemberId() {
    var element = getActiveMemberElement();
    return element.attr('data-id');
}

function watchContentEditableElements() {
    if (contentEditableControl.value == "true") {
        var elements = document.getElementsByTagName('div');
        var element, i;
        var attribute;
        for (i = 0; i < elements.length; i++) {
            element = elements[i];
            watchElement(element);
        }
    }
}

function watchElement(element) {
    if (element.contentEditable == 'true') {
        element.onkeydown = function (event) {
            dokeydown(event);
        }
        element.oncut = function (event) {
            docut(event);
        }
        if (element.isEditableCell == null ||
          element.isEditableCell != true) {
            element.onpaste = function (event) {
                dopaste(event);
            }
        }
    }
}

function dokeydown(event) {
    if (!event) {
        event = window.event;
        var target = event.srcElement;
    }
    else
        var target = event.target;
    // Ignore:
    // Ctrl, Shift, Tab, Home, End, Page Up/Down, Arrow Left/Right/Up/Down
    // Ctrl-A/C
    if (event.keyCode != 16 && event.keyCode != 17 &&
      event.keyCode != 9 &&
      (event.keyCode < 33 || event.keyCode > 40)) {
        if (!event.ctrlKey || ((event.keyCode != 65) && (event.keyCode != 67))) {
            notifyModified();
        }
    }
}

function docut(event) {
    if (!event) {
        event = window.event;
        var target = event.srcElement;
    }
    else
        var target = event.target;

    notifyModified();
}

function dopaste(event) {
    if (!event) {
        event = window.event;
        var target = event.srcElement;
    }
    else
        var target = event.target;

    notifyModified();
}

function AddRowToTable(tableID, className) {
    var table = document.getElementById(tableID);
    if (table != null) {
        AttachRowToTable(table, className);
    }
}

function AttachRowToTable(table, className) {
    var rowCount = table.rows.length;
    var row = table.insertRow(rowCount);

    // Parameter column
    var cellLeft = row.insertCell(0);

    var divTag = document.createElement('div');
    if (className != null)
        divTag.className = className;
    divTag.setAttribute('contentEditable', 'true');
    divTag.setAttribute('tabindex', '0');

    // NOTE: onkeydown/oncut/onpaste will be captured by watchElement function.
    divTag.onkeypress = function () {
        AddSpareTableRow(table, rowCount, className);
    }
    watchElement(divTag);
    divTag.isEditableCell = true;
    divTag.onpaste = function (event) {
        AddSpareTableRow(table, rowCount, className);
        notifyModified();
    }

    cellLeft.appendChild(divTag);

    // Description column
    var cellRight = row.insertCell(1);

    var divTag = document.createElement('div');
    divTag.setAttribute('contentEditable', 'true');
    divTag.setAttribute('tabindex', '0');
    divTag.onkeypress = function () {
        AddSpareTableRow(table, rowCount);
    }
    watchElement(divTag);
    divTag.isEditableCell = true;
    divTag.onpaste = function (event) {
        AddSpareTableRow(table, rowCount);
        notifyModified();
    }
    cellRight.appendChild(divTag);
}

function AddSpareTableRow(table, rowNo, className) {
    if (table != null) {
        if (rowNo == table.rows.length - 1)
            AttachRowToTable(table, className);
    }
}

function RemoveRowFromTable(tableID, row) {
    var table = document.getElementById(tableID);
    if (table != null)
        table.deleteRow(row.rowIndex);
}

function AddRowToDefTables() {
    $('table.definition').each(
		function (index, element) {
		    AttachRowToTable(element);
		}
	);
}

function checkSpareTableRow()
{
    if (document.activeElement) {
        var element = $(document.activeElement).closest('div[contentEditable=true]');
        if (element.hasClass('seealso') || element.hasClass('exception')) { // temp
            var table, className;
            table = element.closest('table')[0];
            className = element.attr('class');
            if (table)
                AttachRowToTable(table, className);
        }
    }
}

// http://stackoverflow.com/questions/6690752
// document.selection is no longer supported.
// Starting with Internet Explorer 11, use getSelection.
function pasteHtmlAtCaret(html, selectPastedContent) {
    var sel, range;
    if (window.getSelection) {
        // IE9 and non-IE
        sel = window.getSelection();
        if (sel.getRangeAt && sel.rangeCount) {
            range = sel.getRangeAt(0);
            range.deleteContents();

            // Range.createContextualFragment() would be useful here but is
            // only relatively recently standardized and is not supported in
            // some browsers (IE9, for one)
            var el = document.createElement("div");
            el.innerHTML = html;
            var frag = document.createDocumentFragment(), node, lastNode;
            while ( (node = el.firstChild) ) {
                lastNode = frag.appendChild(node);
            }
            var firstNode = frag.firstChild;
            range.insertNode(frag);

            // Preserve the selection
            if (lastNode) {
                range = range.cloneRange();
                range.setStartAfter(lastNode);
                if (selectPastedContent) {
                    range.setStartBefore(firstNode);
                } else {
                    range.collapse(true);
                }
                sel.removeAllRanges();
                sel.addRange(range);
            }
        }
    } else if ( (sel = document.selection) && sel.type != "Control") {
        // IE < 9
        var originalRange = sel.createRange();
        originalRange.collapse(true);
        sel.createRange().pasteHTML(html);
        if (selectPastedContent) {
            range = sel.createRange();
            range.setEndPoint("StartToStart", originalRange);
            range.select();
        }
    }
}

function pasteHtml(htmlText) {
    pasteHtmlAtCaret(htmlText, false);
    notifyModified();
    watchContentEditableElements();
}

function insertNote(type, caption, iconUri) {
    var id = CreateGuid();
    var html =
        '<div id="' + id + '" class="note" contentEditable="false">' +
		'<input type="HIDDEN" class="note" value="' + type + '" />' +
		'<table>' +
        '	<tbody>' +
        '		<tr>' +
        '			<th>' +
        '				<img src="' + iconUri + '" class="note" title="Note" />' +
        '				<span class="note">' + caption + '</span>' +
        '			</th>' +
        '		</tr>' +
        '		<tr>' +
        '			<td>' +
        '				<div contentEditable="true">' +
        '					<p>&nbsp;</p>' +
        '				</div>' +
        '			</td>' +
        '		</tr>' +
        '	</tbody>' +
        '</table>' +
        '</div>';
    pasteHtml(html);
    focusEditableContent(id);
}

function insertCode(lang) {
    var id = CreateGuid();
    if (lang == undefined) {
        lang = 'Delphi';
    }
    var caption = "Code";
    var html =
      '<div id="' + id + '" class="codeSnippet" contentEditable="false">' +
      '<input class="code" type="hidden" value="' + lang + '" />' +
      '<div class="codeSnippetTabs">' +
        '<div class="codeSnippetTabActive">' +
          '<a>' + lang + '</a>' +
        '</div>' +
      '</div>' +
      '<div class="codeSnippetCodeContainer">' +
        '<div class="codeSnippetCode">' +
          '<pre class="' + lang + '" contentEditable="true"></pre>' +
        '</div>' +
      '</div>' +
    '</div>';
    pasteHtml(html);
    focusEditableContent(id, 'pre');
}

function S4() {
    return (((1 + Math.random()) * 0x10000) | 0).toString(16).substring(1);
}

function CreateGuid() {
    return (S4() + S4() + "-" + S4() + "-" + S4() + "-" + S4() + "-" + S4() + S4() + S4());
}

function insertDefinitionTable() {
    var id = CreateGuid();
    var html =
        '<div contentEditable="false">' +
        '<table id="' + id + '" class="definition">' +
        '	<tbody>' +
        '		<tr>' +
        '			<th class="termColumn"><div contentEditable="true">Term</div></th>' +
        '			<th class="descriptionColumn"><div contentEditable="true">Description</div></th>' +
        '		</tr>' +
        '		<tr>' +
        '			<td class="term"><div contentEditable="true">&nbsp;</div></td>' +
        '			<td class="description"><div contentEditable="true">&nbsp;</div></td>' +
        '		</tr>' +
        '	</tbody>' +
        '</table></div>';
    pasteHtml(html);
    AddRowToTable(id);
    focusEditableContent(id);
}
