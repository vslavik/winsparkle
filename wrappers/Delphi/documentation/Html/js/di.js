/* 
	Documentation Insight
	Copyright (C) 2010-2013 DevJet Software
*/

var globals;
var showInheritedMembers = true;
var showProtectedMembers = true;

$(function () {
    initializeSections();
    initializeFilters();
});

jQuery.fn.redraw = function(){
  $(this).each(function(){
	var value = this.className;
    this.className = '';
	this.className = value;
  });
};

// get global vars from persistent store			
function GetGlobals() {
    var out;

    // Try to get VS implementation
    try { out = window.external.Globals; }
    catch (e) { out = null; }

    // Try to get DExplore implementation
    try { if (out == null) out = window.external.GetObject("DTE", "").Globals; }
    catch (e) { outk = null; }

    return out;
}

function Save(key, value, days) {
    if (days) {
        var date = new Date();

        date.setTime(date.getTime() + (days * 24 * 60 * 60 * 1000));
        var expires = "; expires=" + date.toGMTString();
    }
    else var expires = "";
    document.cookie = key + "=" + value + expires + "; path=/";
}

function Load(key) {
    var nameEQ = key + "=";
    var ca = document.cookie.split(';');
    for (var i = 0; i < ca.length; i++) {
        var c = ca[i];
        while (c.charAt(0) == ' ') c = c.substring(1, c.length);
        if (c.indexOf(nameEQ) == 0)
            return c.substring(nameEQ.length, c.length);
    }
    return null;
}

function initializeSections() {
    $('.collapsibleAreaHref')
        /*.each(
            function (index, element) {
                var section = $(this).parents('.section');
                setSectionState(section, true);
            }
        ) */
        .bind('click',
            function () {
                var section = $(this).parents('.section');
                var expanded = $('.collapsibleAreaExpanding', this).length > 0;
                setSectionState(section, !expanded);
        });
}

function initializeFilters() {
    // load showInheritedMembers & showProtectedMembers
    $('input.libraryFilterInherited').change(
        function () {
            showInheritedMembers = this.checked;
            updateInheritedMembers();
        }
    );
    $('input.libraryFilterProtected').change(
        function () {
            showProtectedMembers = this.checked;
            updateProtectedMembers();
        }
    );
    $('div.libraryMemberFilter').css('display', 'block');
}

function updateInheritedMembers() {
    $('input.libraryFilterInherited').prop('checked', showInheritedMembers);
    var selector = 'tr.inherited';
    if (!showProtectedMembers) {
        selector = selector + ':not(.protected)';
    }
    if (showInheritedMembers) {
        $(selector).css('display', 'table-row');
    }
    else {
        $(selector).css('display', 'none');
    }
}

function updateProtectedMembers() {
    $('input.libraryFilterProtected').prop('checked', showProtectedMembers);
    var selector = 'tr.protected';
    if (!showInheritedMembers) {
        selector = selector + ':not(.inherited)';
    }
    if (showProtectedMembers) {
        $(selector).css('display', 'table-row');
    }
    else {
        $(selector).css('display', 'none');
    }
}

function setSectionState(section, expanded) {
    var href = $('.collapsibleAreaHref', section)[0];
    var iconSpan = $('.collapsibleAreaIcon', section);
    var sectionContent = $('.sectionContent', section);

    if (!expanded) {
        href.title = 'Expand';
        iconSpan.removeClass('collapsibleAreaExpanding');
        iconSpan.addClass('collapsibleAreaCollapsing');
		iconSpan.redraw();
        sectionContent.css('display', 'none');
    }
    else {
        href.title = 'Collapse';
        iconSpan.addClass('collapsibleAreaExpanding');
        iconSpan.removeClass('collapsibleAreaCollapsing');
        sectionContent.css('display', 'block');
    }
}

function focusEditableContent(id, tag) {
    if (tag == undefined) 
        tag = 'div';
    var element = $('#' + id).find(tag + '[contenteditable="true"]');
    if (element.length) {
        element[0].focus();
    }
}