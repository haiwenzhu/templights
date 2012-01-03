{%% extend template layout.tpl %%}
{% layout.tpl {

{%% override content of block TITLE %%}
{% TITLE {
example.com - <?= $title ?>
}%}

{%% override content of block HEAD %%}
{% HEAD {
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
}%}

{%% override content of block BODY %%}
{% BODY {
    <h1><?= $title; ?></h1>
    <div id="content"><?= $content ?></div>
    <div id="footer">
    
    {%% define block TITLE %%}
    {% FOOTER {
    
    {%% 
    extend template footer.tpl with no blocks defined (template include)
    extra text is appended 
    %%}
    {% ./templates/footer.tpl { Gregor Mazovec }%}
    
    }%}
    </div>
}%}

}%}
